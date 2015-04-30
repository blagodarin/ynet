#include "client.h"

#include <cassert>
#include <vector>

#include "connection.h"

namespace ynet
{
	ClientImpl::ClientImpl(Callbacks& callbacks, const std::function<std::unique_ptr<ConnectionImpl>()>& factory)
		: _callbacks(callbacks)
		, _factory(factory)
		, _thread([this]() { run(); })
	{
	}

	ClientImpl::~ClientImpl()
	{
		assert(_thread.joinable());
		assert(_thread.get_id() != std::this_thread::get_id());
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_stopping = true;
			if (_connection)
			{
				if (_disconnect_timeout == 0)
					_connection->abort();
				else
				{
					_connection->close();
					if (_disconnect_timeout > 0)
					{
						if (!_disconnect_event.wait_for(lock, std::chrono::milliseconds(_disconnect_timeout), [this]() { return !_connection; }))
							_connection->abort();
					}
				}
			}
		}
		_stop_event.notify_one();
		_thread.join();
		assert(!_connection);
	}

	void ClientImpl::run()
	{
		_callbacks.on_started();
		std::vector<uint8_t> receive_buffer;
		for (; ; )
		{
			int reconnect_timeout = -1;
			{
				auto connection = _factory();
				if (connection)
				{
					{
						std::lock_guard<std::mutex> lock(_mutex);
						if (_stopping)
							break;
						_connection = connection.get();
					}
					receive_buffer.resize(connection->receive_buffer_size());
					const std::shared_ptr<Connection> connection_ptr = std::move(connection);
					// Note that the original connection pointer is no longer valid.
					_callbacks.on_connected(connection_ptr);
					for (;;)
					{
						const size_t size = _connection->receive(receive_buffer.data(), receive_buffer.size(), nullptr);
						if (size == 0)
							break;
						_callbacks.on_received(connection_ptr, receive_buffer.data(), size);
					}
					// There is no point in graceful closure at this point
					// because the connection is either closed or broken here.
					_connection->abort();
					{
						std::lock_guard<std::mutex> lock(_mutex);
						_connection = nullptr;
					}
					_callbacks.on_disconnected(connection_ptr, reconnect_timeout);
					_disconnect_event.notify_one();
				}
				else
					_callbacks.on_failed_to_connect(reconnect_timeout);
			}
			if (reconnect_timeout < 0)
				break;
			std::unique_lock<std::mutex> lock(_mutex);
			if (reconnect_timeout > 0)
			{
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(reconnect_timeout), [this]() { return _stopping; }))
					break;
			}
			else if (_stopping)
				break;
		}
	}
}
