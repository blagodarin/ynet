#include "server.h"

#include <cassert>

#include "connection.h"
#include "trigger.h"

namespace ynet
{
	ServerImpl::ServerImpl(Server::Callbacks& callbacks, uint16_t port, Trigger& trigger)
		: _callbacks(callbacks)
		, _sockaddr(make_sockaddr("0.0.0.0", port)) // TODO: Change to :: for an IPv6 server.
		, _address(_sockaddr)
		, _relisten_timeout(1000)
		, _stopping(false)
	{
		trigger = [this]() { _thread = std::thread(std::bind(&ServerImpl::run, this)); };
	}

	ServerImpl::~ServerImpl()
	{
		assert(!_thread.joinable());
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

	void ServerImpl::stop()
	{
		assert(_thread.joinable());
		assert(_thread.get_id() != std::this_thread::get_id());
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_stopping = true;
			shutdown();
		}
		_stop_event.notify_one();
		_thread.join();
	}

	void ServerImpl::run()
	{
		for (bool initial = true; !listen(_sockaddr); )
		{
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
		_callbacks.on_started(*this);
		poll();
		_callbacks.on_stopped(*this);
	}

	void ServerImpl::on_connected(const std::shared_ptr<Connection>& connection)
	{
		_callbacks.on_connected(*this, connection);
	}

	void ServerImpl::on_received(const std::shared_ptr<Connection>& connection, void* buffer, size_t buffer_size, bool& disconnected)
	{
		for (;;)
		{
			const size_t size = static_cast<ConnectionImpl*>(connection.get())->receive(buffer, buffer_size, &disconnected);
			if (size > 0)
				_callbacks.on_received(*this, connection, buffer, size);
			if (size < buffer_size)
				break;
		}
	}

	void ServerImpl::on_disconnected(const std::shared_ptr<Connection>& connection)
	{
		_callbacks.on_disconnected(*this, connection);
	}
}