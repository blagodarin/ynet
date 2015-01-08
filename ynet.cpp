#include "ynet.h"

#include "tcp_client.h"
#include "tcp_server.h"

namespace ynet
{
	void ClientCallbacks::on_started(const std::string&, int)
	{
	}

	void ClientCallbacks::on_refused(const std::string&, int)
	{
	}

	std::unique_ptr<Client> Client::create(ClientCallbacks& callbacks, const std::string& host, int port)
	{
		return std::unique_ptr<Client>(new TcpClient(callbacks, host, port));
	}

	void ServerCallbacks::on_started(const Link&)
	{
	}

	void ServerCallbacks::on_stopped(const Link&)
	{
	}

	std::unique_ptr<Server> Server::create(ServerCallbacks& callbacks, int port)
	{
		return std::unique_ptr<Server>(new TcpServer(callbacks, port));
	}
}
