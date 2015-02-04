#include <ynet.h>

#include "tcp.h"

namespace ynet
{
	void Client::Callbacks::on_started(const Client&)
	{
	}

	void Client::Callbacks::on_failed_to_connect(const Client&)
	{
	}

	void Client::Callbacks::on_stopped(const Client&)
	{
	}

	std::unique_ptr<Client> Client::create(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options)
	{
		std::unique_ptr<ClientImpl> client(new TcpClient(callbacks, host, port, options));
		client->start();
		return std::move(client);
	}

	void Server::Callbacks::on_failed_to_start(const Server&)
	{
	}

	void Server::Callbacks::on_started(const Server&)
	{
	}

	void Server::Callbacks::on_stopped(const Server&)
	{
	}

	std::unique_ptr<Server> Server::create(Callbacks& callbacks, uint16_t port)
	{
		std::unique_ptr<ServerImpl> server(new TcpServer(callbacks, port));
		server->start();
		return std::move(server);
	}
}
