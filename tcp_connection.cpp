#include "tcp_server.h"

namespace ynet
{
	TcpConnection::TcpConnection(TcpBackend::Socket socket, std::string&& address, int port, std::string&& name)
		: _socket(socket)
		, _address(std::move(address))
		, _port(port)
		, _name(std::move(name))
	{
	}

	std::string TcpConnection::address() const
	{
		return _address;
	}

	void TcpConnection::close()
	{
		std::lock_guard<std::mutex> lock(_socket_mutex);
		if (_socket != TcpBackend::InvalidSocket)
		{
			TcpBackend::shutdown(_socket);
			_socket = TcpBackend::InvalidSocket;
		}
	}

	std::string TcpConnection::name() const
	{
		return _name;
	}

	int TcpConnection::port() const
	{
		return _port;
	}

	bool TcpConnection::send(const void* data, size_t size)
	{
		std::lock_guard<std::mutex> lock(_socket_mutex);
		if (_socket == TcpBackend::InvalidSocket)
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
}
