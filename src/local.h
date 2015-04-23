#pragma once

#include <memory>

namespace ynet
{
	class Address;
	class ConnectionImpl;
	class ServerBackend;

	std::unique_ptr<ConnectionImpl> create_local_connection(uint16_t port);
	std::unique_ptr<ServerBackend> create_local_server(const Address& address);
}
