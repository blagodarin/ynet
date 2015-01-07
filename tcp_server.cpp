#include "tcp_server.h"

namespace ynet
{
	TcpServer::TcpServer(ServerCallback& callback, int port)
		: _callback(callback)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _port_string(_port >= 0 ? std::to_string(_port) : std::string())
	{
	}

	TcpServer::~TcpServer()
	{
	}

	bool TcpServer::start()
	{
		return false;
	}
}
