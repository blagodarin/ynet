#include <ynet.h>

#include "client.h"
#include "connection.h"
#include "local.h"
#include "server.h"
#include "tcp.h"

// TODO: Add Windows port.

namespace ynet
{
	void Client::Callbacks::on_started()
	{
	}

	void Client::Callbacks::on_stopped()
	{
	}

	std::unique_ptr<Client> Client::create_local(Callbacks& callbacks, const std::string& name)
	{
		return std::make_unique<ClientImpl>(callbacks, [name]() { return create_local_connection(name); });
	}

	std::unique_ptr<Client> Client::create_tcp(Callbacks& callbacks, const std::string& host, uint16_t port)
	{
		return std::make_unique<ClientImpl>(callbacks, [host, port]() { return create_tcp_connection(host, port); });
	}

	void Server::Callbacks::on_started()
	{
	}

	std::unique_ptr<Server> Server::create_local(Callbacks& callbacks, const std::string& name)
	{
		return std::make_unique<ServerImpl>(callbacks, [name]() { return create_local_server(name); });
	}

	std::unique_ptr<Server> Server::create_tcp(Callbacks& callbacks, uint16_t port)
	{
		return std::make_unique<ServerImpl>(callbacks, [port]() { return create_tcp_server(port); });
	}
}
