#include "server.h"

#include <cassert>

#include "connection.h"
#include "local.h"
#include "tcp.h"

namespace ynet
{
	Server::Options::Options()
		: optimized_loopback(true)
	{
	}

	ServerImpl::ServerImpl(Server::Callbacks& callbacks, uint16_t port, const Options& options)
		: _callbacks(callbacks)
		, _handlers(*this, _callbacks)
		, _options(options)
		, _relisten_timeout(1000)
		, _sockaddr(make_sockaddr("0.0.0.0", port)) // TODO: Change to :: for an IPv6 server.
		, _address(_sockaddr)
		, _thread(std::bind(&ServerImpl::run, this))
	{
	}

	ServerImpl::~ServerImpl()
	{
		assert(_thread.joinable());
		assert(_thread.get_id() != std::this_thread::get_id());
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_stopping = true;
			if (_backend)
			{
				_backend->shutdown();
				_backend = nullptr;
			}
			if (_local_backend)
			{
				_local_backend->shutdown();
				_local_backend = nullptr;
			}
		}
		_stop_event.notify_one();
		_thread.join();
	}

	std::string ServerImpl::address() const
	{
		return _address._address;
	}

	std::string ServerImpl::name() const
	{
		return _address._name;
	}

	uint16_t ServerImpl::port() const
	{
		return _address._port;
	}

	void ServerImpl::run()
	{
		std::unique_ptr<ServerBackend> backend;
		for (bool initial = true; ; )
		{
			backend = create_tcp_server(_sockaddr);
			if (backend)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_backend = backend.get();
				break;
			}
			if (initial)
			{
				initial = false;
				_callbacks.on_failed_to_start(*this);
			}
			std::unique_lock<std::mutex> lock(_mutex);
			if (_relisten_timeout > 0)
			{
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(_relisten_timeout), [this]() { return _stopping; }))
					return;
			}
			else if (_stopping)
				return;
		}

		std::thread local_thread;
		if (_options.optimized_loopback)
		{
			local_thread = std::thread(std::bind(&ServerImpl::run_local, this));
			std::unique_lock<std::mutex> lock(_mutex);
			_local_server_started.wait(lock, [this]() { return _local_backend || _stopping; });
			if (_stopping)
			{
				lock.unlock();
				local_thread.join();
				return;
			}
		}

		_callbacks.on_started(*this);

		if (_options.optimized_loopback)
		{
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_start_local_polling = true;
			}
			_start_local_polling_condition.notify_one();
		}

		backend->poll(_handlers);

		if (_options.optimized_loopback)
			local_thread.join();

		_callbacks.on_stopped(*this);
	}

	void ServerImpl::run_local()
	{
		std::unique_ptr<ServerBackend> backend;
		for (; ; )
		{
			backend = create_local_server(_address._port, _sockaddr.ss_family == AF_INET6);
			if (backend)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_local_backend = backend.get();
				break;
			}
			std::unique_lock<std::mutex> lock(_mutex);
			if (_relisten_timeout > 0)
			{
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(_relisten_timeout), [this]() { return _stopping; }))
					break;
			}
			else if (_stopping)
				break;
		}

		_local_server_started.notify_one();
		if (!backend)
			return;

		{
			std::unique_lock<std::mutex> lock(_mutex);
			_start_local_polling_condition.wait(lock, [this]() { return _start_local_polling || _stopping; });
			if (_stopping)
				return;
		}

		backend->poll(_handlers);
	}
}
