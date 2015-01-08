#pragma once

#include <string>

#include "ynet.h"

namespace ynet
{
	namespace TcpBackend
	{
		typedef int Socket;

		const Socket InvalidSocket = -1;

		Socket accept(Socket socket, std::string& address, int& port);
		void close(Socket socket);
		Socket connect(const std::string& host, const std::string& port, Link& link);
		Socket listen(int port, std::string& address);
		size_t recv(Socket socket, void* data, size_t size);
		bool send(Socket socket, const void* data, size_t size);
		void shutdown(Socket socket);
	}
}
