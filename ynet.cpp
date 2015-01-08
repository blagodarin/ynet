#include "ynet.h"

#include "tcp_client.h"
#include "tcp_server.h"

namespace ynet
{
	void ClientCallback::on_started(const std::string&, int)
	{
	}

	void ClientCallback::on_refused(const std::string&, int)
	{
	}

	std::unique_ptr<Client> Client::create(ClientCallback& callback, const std::string& host, int port)
	{
		return std::unique_ptr<Client>(new TcpClient(callback, host, port));
	}

	void ServerCallback::on_started(const Link&)
	{
	}

	std::unique_ptr<Server> Server::create(ServerCallback& callback, int port)
	{
		return std::unique_ptr<Server>(new TcpServer(callback, port));
	}
}
