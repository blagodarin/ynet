#pragma once

#include <memory>

namespace ynet
{
	class ConnectionImpl;
	class ServerBackend;

	std::unique_ptr<ConnectionImpl> create_local_connection(uint16_t port);
	std::unique_ptr<ServerBackend> create_local_server(uint16_t port);
}
