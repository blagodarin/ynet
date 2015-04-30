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

	std::unique_ptr<Client> Client::create_local(Callbacks& callbacks, uint16_t port)
	{
		return std::make_unique<ClientImpl>(callbacks, [port]() { return create_local_connection(port); });
	}

	std::unique_ptr<Client> Client::create_tcp(Callbacks& callbacks, const std::string& host, uint16_t port)
	{
		return std::make_unique<ClientImpl>(callbacks, [host, port]() { return create_tcp_connection(host, port); });
	}

	void Server::Callbacks::on_started()
	{
	}

	std::unique_ptr<Server> Server::create_local(Callbacks& callbacks, uint16_t port)
	{
		return std::make_unique<ServerImpl>(callbacks, [port]() { return create_local_server(port); });
	}

	std::unique_ptr<Server> Server::create_tcp(Callbacks& callbacks, uint16_t port)
	{
		return std::make_unique<ServerImpl>(callbacks, [port]() { return create_tcp_server(port); });
	}
}
