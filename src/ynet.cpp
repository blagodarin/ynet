#include <ynet.h>

#include "backend.h"
#include "trigger.h"

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

	std::unique_ptr<Client> Client::create(Callbacks& callbacks, const std::string& host, uint16_t port)
	{
		Trigger trigger;
		return std::unique_ptr<Client>(new TcpClient(callbacks, host, port, trigger));
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
		Trigger trigger;
		return std::unique_ptr<Server>(new TcpServer(callbacks, port, trigger));
	}
}
