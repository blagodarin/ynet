#include "server.h"

#include <cassert>

#include "connection.h"
#include "local.h"
#include "tcp.h"

// TODO: Split the "normal" TCP server and the optimized loopback server into two separate classes.

namespace ynet
{
	Server::Options::Options()
		: optimized_loopback(true)
	{
	}

	ServerImpl::ServerImpl(Callbacks& callbacks, uint16_t port, const Options& options)
		: _callbacks(callbacks)
		, _handlers(_callbacks)
		, _options(options)
		, _address(Address::Family::IPv4, Address::Special::Any, port)
		, _thread([this]() { run(); })
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

	void ServerImpl::run()
	{
		std::unique_ptr<ServerBackend> backend;
		for (; ; )
		{
			backend = create_tcp_server(_address);
			if (backend)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				if (_stopping)
					return;
				_backend = backend.get();
				break;
			}
			int restart_timeout = -1;
			_callbacks.on_failed_to_start(restart_timeout);
			if (restart_timeout < 0)
				return;
			std::unique_lock<std::mutex> lock(_mutex);
			if (restart_timeout > 0)
			{
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(restart_timeout), [this]() { return _stopping; }))
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

		_callbacks.on_started();

		if (_options.optimized_loopback)
		{
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_start_local_polling = true;
			}
			_start_local_polling_condition.notify_one();
		}

		backend->run(_handlers);

		if (_options.optimized_loopback)
			local_thread.join();

		_callbacks.on_stopped();
	}

	void ServerImpl::run_local()
	{
		const auto backend = create_local_server(_address);
		if (backend)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_local_backend = backend.get();
		}
		_local_server_started.notify_one();
		if (!backend)
		{
			// A local server is only started after a network one has.
			// Startup failure means there are external reasons preventing the corresponding local server to work.
			// Treating such failure as fatal is the simplest option, we already have a network server anyway.
			return;
		}

		{
			std::unique_lock<std::mutex> lock(_mutex);
			_start_local_polling_condition.wait(lock, [this]() { return _start_local_polling || _stopping; });
			if (_stopping)
				return;
		}

		backend->run(_handlers);
	}
}
