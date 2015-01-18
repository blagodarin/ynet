#include "client.h"

#include <cassert>

#include "connection.h"
#include "trigger.h"

namespace ynet
{
	ClientImpl::ClientImpl(Callbacks& callbacks, const std::string& host, uint16_t port, Trigger& trigger)
		: _callbacks(callbacks)
		, _host(host)
		, _port(port)
		, _port_string(std::to_string(_port))
		, _reconnect_timeout(1000)
		, _connection(nullptr)
		, _stopping(false)
	{
		trigger = [this]() { _thread = std::thread(std::bind(&ClientImpl::run, this)); };
	}

	ClientImpl::~ClientImpl()
	{
		assert(!_thread.joinable());
	}

	std::string ClientImpl::host() const
	{
		return _host;
	}

	uint16_t ClientImpl::port() const
	{
		return _port;
	}

	void ClientImpl::stop()
	{
		assert(_thread.joinable());
		assert(_thread.get_id() != std::this_thread::get_id());
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_stopping = true;
			if (_connection)
				_connection->close();
		}
		_stop_event.notify_one();
		_thread.join();
		assert(!_connection);
	}

	void ClientImpl::run()
	{
		const auto resolve_and_connect = [this]() -> std::shared_ptr<Connection>
		{
			// TODO: Get rid of an explicit TCP here.
			for (const auto& address : resolve(_host, Protocol::Tcp, _port_string))
			{
				const auto& connection = connect(address);
				if (connection)
					return connection;
			}
			return {};
		};

		_callbacks.on_started(*this);
		std::vector<uint8_t> buffer(receive_buffer_size());
		for (bool initial = true; ; )
		{
			{
				const auto& connection = resolve_and_connect();
				if (connection)
				{
					initial = false;
					{
						std::lock_guard<std::mutex> lock(_mutex);
						if (_stopping)
							break;
						_connection = static_cast<ConnectionImpl*>(connection.get());
					}
					_callbacks.on_connected(*this, connection);
					for (;;)
					{
						const size_t size = _connection->receive(buffer.data(), buffer.size(), nullptr);
						if (size == 0)
							break;
						_callbacks.on_received(*this, connection, buffer.data(), size);
					}
					connection->close();
					{
						std::lock_guard<std::mutex> lock(_mutex);
						_connection = nullptr;
					}
					_callbacks.on_disconnected(*this, connection);
				}
				else if (initial)
				{
					initial = false;
					_callbacks.on_failed_to_connect(*this);
				}
			}
			std::unique_lock<std::mutex> lock(_mutex);
			if (_reconnect_timeout > 0)
			{
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(_reconnect_timeout), [this]() { return _stopping; }))
					break;
			}
			else if (_stopping)
				break;
		}
		_callbacks.on_stopped(*this);
	}
}
