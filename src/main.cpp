#include <ynet.h>

#include "client.h"
#include "server.h"

// TODO: Add Windows port.

namespace ynet
{
	void Client::Callbacks::on_started()
	{
	}

	void Client::Callbacks::on_stopped()
	{
	}

	std::unique_ptr<Client> Client::create(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options)
	{
		return std::make_unique<ClientImpl>(callbacks, host, port, options);
	}

	void Server::Callbacks::on_started()
	{
	}

	void Server::Callbacks::on_stopped()
	{
	}

	std::unique_ptr<Server> Server::create(Callbacks& callbacks, uint16_t port, const Options& options)
	{
		return std::make_unique<ServerImpl>(callbacks, port, options);
	}
}
