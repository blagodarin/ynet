#include "client.h"

#include <cassert>

#include "local.h"
#include "tcp.h"

namespace ynet
{
	Client::Options::Options()
		: reconnect_timeout(1000)
		, optimized_loopback(true)
	{
	}

	ClientImpl::ClientImpl(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options)
		: _callbacks(callbacks)
		, _host(host)
		, _port(port)
		, _options(options)
		, _connection(nullptr)
		, _stopping(false)
		, _thread(std::bind(&ClientImpl::run, this))
	{
	}

	ClientImpl::~ClientImpl()
	{
		assert(_thread.joinable());
		assert(_thread.get_id() != std::this_thread::get_id());
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_stopping = true;
			if (_connection)
				_connection->close();
			// TODO: Consider aborting the connection here instead of gracefully closing it.
			// The current implementation hangs if the server is constantly sending us data
			// and doesn't check for the client to become exhausted.
		}
		_stop_event.notify_one();
		_thread.join();
		assert(!_connection);
	}

	std::string ClientImpl::host() const
	{
		return _host;
	}

	uint16_t ClientImpl::port() const
	{
		return _port;
	}

	void ClientImpl::run()
	{
		const auto resolve_and_connect = [this]() -> std::unique_ptr<ConnectionImpl>
		{
			const auto& resolved = resolve(_host, _port);
			if (resolved.local)
			{
				auto connection = create_local_connection(_port);
				if (connection)
					return connection;
			}
			for (const auto& address : resolved.addresses)
			{
				auto connection = create_tcp_connection(address);
				if (connection)
					return connection;
			}
			return {};
		};

		_callbacks.on_started(*this);
		std::vector<uint8_t> receive_buffer;
		for (bool initial = true; ; )
		{
			{
				auto connection = resolve_and_connect();
				if (connection)
				{
					initial = false;
					{
						std::lock_guard<std::mutex> lock(_mutex);
						if (_stopping)
							break;
						_connection = connection.get();
					}
					receive_buffer.resize(connection->receive_buffer_size());
					const std::shared_ptr<Connection> connection_ptr = std::move(connection);
					_callbacks.on_connected(*this, connection_ptr);
					for (;;)
					{
						const size_t size = _connection->receive(receive_buffer.data(), receive_buffer.size(), nullptr);
						if (size == 0)
							break;
						_callbacks.on_received(*this, connection_ptr, receive_buffer.data(), size);
					}
					connection_ptr->close();
					{
						std::lock_guard<std::mutex> lock(_mutex);
						_connection = nullptr;
					}
					_callbacks.on_disconnected(*this, connection_ptr);
				}
				else if (initial)
				{
					initial = false;
					_callbacks.on_failed_to_connect(*this);
				}
			}
			std::unique_lock<std::mutex> lock(_mutex);
			if (_options.reconnect_timeout > 0)
			{
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(_options.reconnect_timeout), [this]() { return _stopping; }))
					break;
			}
			else if (_stopping)
				break;
		}
		_callbacks.on_stopped(*this);
	}
}
