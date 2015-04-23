#pragma once

#include <memory>

namespace ynet
{
	class Address;
	class ConnectionImpl;
	class ServerBackend;

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const Address& address);
	std::unique_ptr<ServerBackend> create_tcp_server(const Address& address);
}
