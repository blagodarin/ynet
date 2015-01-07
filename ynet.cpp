#include "ynet.h"

#include "tcp_client.h"

namespace ynet
{
	std::unique_ptr<Client> Client::create(ClientCallback& callback, const std::string& host, int port)
	{
		return std::unique_ptr<Client>(new TcpClient(callback, host, port));
	}

	std::unique_ptr<Server> Server::create(ServerCallback& callback, int port)
	{
		return std::unique_ptr<Server>(nullptr);
	}
}
