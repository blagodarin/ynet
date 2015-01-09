#pragma once

#include <map>
#include <thread>

#include "tcp_backend.h"

namespace ynet
{
	class TcpServer: public Server, private TcpBackend::Poller::Callbacks
	{
	public:

		TcpServer(ServerCallbacks& callbacks, int port);
		~TcpServer() override;

		// Server
		std::string address() const override;
		int port() const override;

	private:

		void run();

		// TcpBackend::Poller::Callbacks
		void on_connected(TcpBackend::Socket socket, std::string&& address, int port) override;
		void on_received(TcpBackend::Socket socket, bool& disconnected) override;
		void on_disconnected(TcpBackend::Socket socket) override;

	private:

		ServerCallbacks& _callbacks;
		const int _port;
		std::string _address;
		TcpBackend::Socket _socket;
		std::map<TcpBackend::Socket, std::pair<std::string, int>> _peers;
		std::thread _thread;
		TcpBackend::Poller _poller;
		std::array<unsigned char, 48 * 1024> _buffer;
	};
}
