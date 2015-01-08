#include "tcp_server.h"

namespace ynet
{
	class TcpServerSocket: public Socket
	{
	public:

		TcpServerSocket(TcpBackend::Socket socket)
			: _socket(socket)
		{
		}

		~TcpServerSocket() override
		{
			TcpBackend::close(_socket);
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
		TcpBackend::close(_socket); // TODO: ???
		_thread.join();
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
		for (;;)
		{
			const auto socket = TcpBackend::accept(_socket, link.remote_address, link.remote_port);
			if (socket == TcpBackend::InvalidSocket)
				break;
			{
				TcpServerSocket server_socket(socket);
				_callback.on_connected(link, server_socket);
				for (;;)
				{
					const size_t size = TcpBackend::recv(socket, _buffer.data(), _buffer.size());
					if (size == 0)
						break;
					_callback.on_received(link, _buffer.data(), size, server_socket);
				}
			}
			_callback.on_disconnected(link);
		}
	}
}
