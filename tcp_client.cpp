#include "tcp_client.h"

#include <cassert>

#include "tcp_connection.h"

namespace ynet
{
	TcpClient::TcpClient(Callbacks& callbacks, const std::string& host, int port)
		: _callbacks(callbacks)
		, _host(host)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _port_string(_port >= 0 ? std::to_string(_port) : std::string())
		, _reconnect_timeout(1000)
		, _connection(nullptr)
		, _stopping(false)
	{
		if (_port < 0)
			return; // TODO: Throw.
		_thread = std::thread(std::bind(&TcpClient::run, this));
	}

	TcpClient::~TcpClient()
	{
		if (_thread.joinable()) // TODO: Assert.
		{
			assert(_thread.get_id() != std::this_thread::get_id());
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_stopping = true;
				if (_connection)
					_connection->close();
			}
			_stop_event.notify_one();
			_thread.join();
		}
		assert(!_connection);
	}

	std::string TcpClient::host() const
	{
		return _host;
	}

	int TcpClient::port() const
	{
		return _port;
	}

	void TcpClient::run()
	{
		_callbacks.on_started(*this);
		std::array<unsigned char, ReceiveBlockSize> buffer;
		for (bool initial = true; ; )
		{
			std::string address;
			std::string name;
			const auto socket = TcpBackend::connect(_host, _port_string, address, name);
			if (socket != TcpBackend::InvalidSocket)
			{
				initial = false;
				const auto& connection = std::make_shared<TcpConnection>(socket, std::move(address), _port, std::move(name));
				{
					std::lock_guard<std::mutex> lock(_mutex);
					if (_stopping)
					{
						TcpBackend::close(socket);
						break;
					}
					_connection = connection.get();
				}
				_callbacks.on_connected(*this, connection);
				for (;;)
				{
					const size_t size = TcpBackend::recv(socket, buffer.data(), buffer.size(), nullptr);
					if (size == 0)
						break;
					_callbacks.on_received(*this, connection, buffer.data(), size);
				}
				connection->close();
				TcpBackend::close(socket);
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
