#pragma once

#include <thread>

#include "tcp_backend.h"

namespace ynet
{
	class TcpServer: public Server
	{
	public:

		TcpServer(ServerCallback& callback, int port);
		~TcpServer() override;

		// Server
		bool start() override;

	private:

		void run();

	private:

		ServerCallback& _callback;
		const int _port;
		std::string _address;
		TcpBackend::Socket _socket;
		std::thread _thread;
		std::array<unsigned char, 48 * 1024> _buffer;
	};
}
