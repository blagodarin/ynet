#include <string>

#include "ynet.h"

namespace ynet
{
	namespace TcpClientBackend
	{
		typedef int Socket;

		const Socket InvalidSocket = -1;

		void close(Socket socket);
		Socket connect(const std::string& host, const std::string& port, Link& link);
		size_t recv(Socket socket, void* data, size_t size);
		bool send(Socket socket, const void* data, size_t size);
		void shutdown(Socket socket);
	}
}
