#include "tcp_client.h"

#include <cassert>

namespace ynet
{
	TcpClient::TcpClient(ClientCallbacks& callbacks, const std::string& host, int port)
		: _callbacks(callbacks)
		, _host(host)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _port_string(_port >= 0 ? std::to_string(_port) : std::string())
		, _reconnect_timeout(1000)
		, _socket(TcpBackend::InvalidSocket)
		, _shutting_down(false)
		, _closing(false)
	{
		if (_port < 0)
			return; // TODO: Throw.
		_thread = std::thread(std::bind(&TcpClient::run, this));
	}

	TcpClient::~TcpClient()
	{
		if (_thread.joinable())
		{
			assert(_thread.get_id() != std::this_thread::get_id());
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_closing = true;
				if (_socket != TcpBackend::InvalidSocket && !_shutting_down)
				{
					TcpBackend::shutdown(_socket);
					_shutting_down = true;
				}
			}
			_closing_event.notify_one();
			_thread.join();
		}
		assert(_socket == TcpBackend::InvalidSocket);
	}

	std::string TcpClient::address() const
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _address;
	}

	void TcpClient::disconnect()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_shutting_down)
		{
			TcpBackend::shutdown(_socket);
			_shutting_down = true;
		}
	}

	std::string TcpClient::host() const
	{
		return _host;
	}

	int TcpClient::port() const
	{
		return _port;
	}

	bool TcpClient::send(const void* data, size_t size)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_socket == TcpBackend::InvalidSocket || _shutting_down)
			return false;
		auto block = static_cast<const uint8_t*>(data);
		while (size > 0)
		{
			const size_t block_size = std::min(_buffer.size(), size);
			if (!TcpBackend::send(_socket, block, block_size))
				return false;
			block += block_size;
			size -= block_size;
		}
		return true;
	}

	void TcpClient::run()
	{
		_callbacks.on_started(*this);
		for (bool initial = true; ; )
		{
			std::string address;
			const auto socket = TcpBackend::connect(_host, _port_string, address);
			if (socket != TcpBackend::InvalidSocket)
			{
				initial = false;
				{
					std::lock_guard<std::mutex> lock(_mutex);
					if (_closing)
					{
						TcpBackend::close(socket);
						break;
					}
					_socket = socket;
					_address = address;
				}
				_callbacks.on_connected(*this);
				for (;;)
				{
					const size_t size = TcpBackend::recv(_socket, _buffer.data(), _buffer.size(), nullptr);
					if (size == 0)
						break;
					_callbacks.on_received(*this, _buffer.data(), size);
				}
				{
					std::lock_guard<std::mutex> lock(_mutex);
					TcpBackend::close(_socket);
					_socket = TcpBackend::InvalidSocket;
					_shutting_down = false;
				}
				_callbacks.on_disconnected(*this);
			}
			else if (initial)
			{
				initial = false;
				_callbacks.on_failed_to_connect(*this);
			}

			std::unique_lock<std::mutex> lock(_mutex);
			_address.clear();
			if (_reconnect_timeout > 0)
			{
				if (_closing_event.wait_for(lock, std::chrono::milliseconds(_reconnect_timeout), [this]() { return _closing; }))
					break;
			}
			else if (_closing)
				break;
		}
		_callbacks.on_stopped(*this);
	}
}
