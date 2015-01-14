#pragma once

#include <map>
#include <thread>

#include "tcp_connection.h"

namespace ynet
{
	class TcpServer: public Server, private TcpBackend::Poller::Callbacks
	{
	public:

		TcpServer(Server::Callbacks& callbacks, int port);
		~TcpServer() override;

		std::string address() const override;
		std::string name() const override;
		int port() const override;

	private:

		void run();

		void on_connected(TcpBackend::Socket socket, std::string&& address, int port, std::string&& name) override;
		void on_received(TcpBackend::Socket socket, bool& disconnected) override;
		void on_disconnected(TcpBackend::Socket socket) override;

	private:

		Server::Callbacks& _callbacks;
		const int _port;
		std::string _address;
		std::string _name;
		TcpBackend::Socket _socket;
		std::map<TcpBackend::Socket, std::shared_ptr<TcpConnection>> _connections;
		std::thread _thread;
		TcpBackend::Poller _poller;
		std::array<unsigned char, ReceiveBlockSize> _buffer;
	};
}
