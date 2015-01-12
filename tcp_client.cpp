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
		, _disconnecting(false)
		, _stopping(false)
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
				_stopping = true;
				if (_socket != TcpBackend::InvalidSocket && !_disconnecting)
				{
					TcpBackend::shutdown(_socket);
					_disconnecting = true;
				}
			}
			_stop_event.notify_one();
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
		if (!_disconnecting)
		{
			TcpBackend::shutdown(_socket);
			_disconnecting = true;
		}
	}

	std::string TcpClient::host() const
	{
		return _host;
	}

	std::string TcpClient::name() const
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _name;
	}

	int TcpClient::port() const
	{
		return _port;
	}

	bool TcpClient::send(const void* data, size_t size)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_socket == TcpBackend::InvalidSocket || _disconnecting)
			return false;
		auto block = static_cast<const uint8_t*>(data);
		while (size > 0)
		{
			const size_t block_size = std::min(SendBlockSize, size);
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
		std::array<unsigned char, ReceiveBlockSize> buffer;
		for (bool initial = true; ; )
		{
			std::string address;
			std::string name;
			const auto socket = TcpBackend::connect(_host, _port_string, address, name);
			if (socket != TcpBackend::InvalidSocket)
			{
				initial = false;
				{
					std::lock_guard<std::mutex> lock(_mutex);
					if (_stopping)
					{
						TcpBackend::close(socket);
						break;
					}
					_socket = socket;
					_address = std::move(address);
					_name = std::move(name);
				}
				_callbacks.on_connected(*this);
				for (;;)
				{
					const size_t size = TcpBackend::recv(_socket, buffer.data(), buffer.size(), nullptr);
					if (size == 0)
						break;
					_callbacks.on_received(*this, buffer.data(), size);
				}
				{
					std::lock_guard<std::mutex> lock(_mutex);
					TcpBackend::close(_socket);
					_socket = TcpBackend::InvalidSocket;
					_disconnecting = false;
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
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(_reconnect_timeout), [this]() { return _stopping; }))
					break;
			}
			else if (_stopping)
				break;
		}
		_callbacks.on_stopped(*this);
	}
}
