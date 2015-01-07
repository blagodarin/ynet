#include "tcp_client.h"

#include <cassert>

namespace ynet
{
	TcpClient::TcpClient(ClientCallback& callback, const std::string& host, int port)
		: _callback(callback)
		, _host(host)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _port_string(_port >= 0 ? std::to_string(_port) : std::string())
		, _open(false)
		, _socket(TcpClientBackend::InvalidSocket)
		, _closing(false)
	{
	}

	TcpClient::~TcpClient()
	{
		close();
	}

	bool TcpClient::open()
	{
		if (_open || _port < 0)
			return false;
		_open = true;
		_thread = std::thread(std::bind(&TcpClient::run, this));
		return true;
	}

	void TcpClient::close()
	{
		// TODO: Fix closing from the client thread.
		if (!_open)
			return;
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_closing = true;
			if (_socket != TcpClientBackend::InvalidSocket)
				TcpClientBackend::shutdown(_socket);
		}
		_closing_event.notify_one();
		_thread.join();
		_open = false;
		assert(_socket == TcpClientBackend::InvalidSocket);
		_closing = false;
	}

	bool TcpClient::send(const void* data, size_t size)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_socket == TcpClientBackend::InvalidSocket || _closing)
			return false;
		auto block = static_cast<const uint8_t*>(data);
		while (size > 0)
		{
			const size_t block_size = std::min(_buffer.size(), size);
			if (!TcpClientBackend::send(_socket, block, block_size))
				return false;
			block += block_size;
			size -= block_size;
		}
		return true;
	}

	std::string TcpClient::remote_host() const
	{
		return _host;
	}

	int TcpClient::remote_port() const
	{
		return _port;
	}

	void TcpClient::run()
	{
		ClientConnection connection;
		for (bool initial = true; ; )
		{
			const auto socket = TcpClientBackend::connect(_host, _port_string, connection);
			if (socket != TcpClientBackend::InvalidSocket)
			{
				initial = false;
				{
					std::lock_guard<std::mutex> lock(_mutex);
					if (_closing)
					{
						TcpClientBackend::close(socket);
						return;
					}
					_socket = socket;
				}
				_callback.on_connect(*this, connection);
				for (;;)
				{
					const size_t size = TcpClientBackend::recv(_socket, _buffer.data(), _buffer.size());
					if (size == 0)
						break;
					_callback.on_receive(*this, connection, _buffer.data(), size);
				}
				{
					std::lock_guard<std::mutex> lock(_mutex);
					TcpClientBackend::close(_socket);
					_socket = TcpClientBackend::InvalidSocket;
				}
				_callback.on_disconnect(*this, connection);
			}
			else if (initial)
			{
				initial = false;
				_callback.on_refuse(*this);
			}

			std::unique_lock<std::mutex> lock(_mutex);
			if (_closing_event.wait_for(lock, std::chrono::seconds(1), [this]() { return _closing; }))
				return;
		}
	}
}
