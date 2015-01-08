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

		void close() override
		{
			// TODO: Implement.
		}

		bool send(const void* data, size_t size) override
		{
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

	TcpServer::TcpServer(ServerCallback& callback, int port)
		: _callback(callback)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _socket(TcpBackend::InvalidSocket)
	{
	}

	TcpServer::~TcpServer()
	{
		if (_socket == TcpBackend::InvalidSocket)
			return;
		TcpBackend::shutdown_server(_socket);
		_thread.join();
		TcpBackend::close(_socket);
	}

	bool TcpServer::start()
	{
		if (_socket != TcpBackend::InvalidSocket || _port < 0)
			return false;
		_socket = TcpBackend::listen(_port, _address);
		if (_socket == TcpBackend::InvalidSocket)
			return false;
		_thread = std::thread(std::bind(&TcpServer::run, this));
		return true;
	}

	void TcpServer::run()
	{
		Link link;
		link.local_address = _address;
		link.local_port = _port;
		_callback.on_started(link);
		TcpBackend::Poller poller(_socket, *this);
		while (poller)
			poller.poll();
		_callback.on_stopped(link);
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
		_callback.on_connected(link, server_socket);
	}

	void TcpServer::on_received(TcpBackend::Socket socket, bool& disconnected)
	{
		const auto peer = _peers.find(socket);
		assert(peer != _peers.end());
		const Link& peer_link = peer->second;
		for (;;)
		{
			const size_t size = TcpBackend::recv(socket, _buffer.data(), _buffer.size(), &disconnected);
			if (size > 0)
			{
				TcpServerSocket server_socket(socket);
				_callback.on_received(peer_link, _buffer.data(), size, server_socket);
			}
			if (size < _buffer.size())
				break;
		}
	}

	void TcpServer::on_disconnected(TcpBackend::Socket socket)
	{
		const auto peer = _peers.find(socket);
		assert(peer != _peers.end());
		_callback.on_disconnected(peer->second);
		_peers.erase(peer);
	}
}
