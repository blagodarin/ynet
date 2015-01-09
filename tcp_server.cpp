#include "tcp_server.h"

#include <cassert>

namespace ynet
{
	class TcpServerSocket: public Socket
	{
	public:

		TcpServerSocket(TcpBackend::Socket socket)
			: _socket(socket)
		{
		}

		explicit operator bool() const
		{
			return _socket != TcpBackend::InvalidSocket;
		}

		void close() override
		{
			if (_socket != TcpBackend::InvalidSocket)
			{
				TcpBackend::shutdown(_socket);
				_socket = TcpBackend::InvalidSocket;
			}
		}

		bool send(const void* data, size_t size) override
		{
			if (_socket == TcpBackend::InvalidSocket)
				return false;
			auto block = static_cast<const uint8_t*>(data);
			while (size > 0)
			{
				const size_t block_size = std::min<size_t>(48 * 1024, size);
				if (!TcpBackend::send(_socket, block, block_size))
					return false;
				block += block_size;
				size -= block_size;
			}
			return true;
		}

	private:

		TcpBackend::Socket _socket;
	};

	TcpServer::TcpServer(ServerCallbacks& callbacks, int port)
		: _callbacks(callbacks)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _socket(TcpBackend::InvalidSocket)
		, _poller(*this)
	{
		if (_port < 0)
			return; // TODO: Throw.
		_socket = TcpBackend::listen(_port, _address);
		if (_socket == TcpBackend::InvalidSocket)
			return; // TODO: Retry.
		_thread = std::thread(std::bind(&TcpServer::run, this));
	}

	TcpServer::~TcpServer()
	{
		if (_socket == TcpBackend::InvalidSocket)
			return;
		TcpBackend::shutdown_server(_socket);
		_thread.join();
		TcpBackend::close(_socket);
	}

	std::string TcpServer::address() const
	{
		return _address;
	}

	int TcpServer::port() const
	{
		return _port;
	}

	void TcpServer::run()
	{
		_callbacks.on_started(*this);
		_poller.run(_socket);
		_callbacks.on_stopped(*this);
	}

	void TcpServer::on_connected(TcpBackend::Socket socket, std::string&& address, int port)
	{
		Link link;
		link.local_address = _address;
		link.local_port = _port;
		link.remote_address = address;
		link.remote_port = port;
		_peers.emplace(socket, link);
		TcpServerSocket server_socket(socket);
		_callbacks.on_connected(*this, link.remote_address, link.remote_port, server_socket);
	}

	void TcpServer::on_received(TcpBackend::Socket socket, bool& disconnected)
	{
		const auto peer = _peers.find(socket);
		assert(peer != _peers.end());
		for (TcpServerSocket server_socket(socket); server_socket; )
		{
			const size_t size = TcpBackend::recv(socket, _buffer.data(), _buffer.size(), &disconnected);
			if (size > 0)
				_callbacks.on_received(*this, peer->second.remote_address, peer->second.remote_port, _buffer.data(), size, server_socket);
			if (size < _buffer.size())
				break;
		}
	}

	void TcpServer::on_disconnected(TcpBackend::Socket socket)
	{
		const auto peer = _peers.find(socket);
		assert(peer != _peers.end());
		_callbacks.on_disconnected(*this, peer->second.remote_address, peer->second.remote_port);
		_peers.erase(peer);
	}
}
