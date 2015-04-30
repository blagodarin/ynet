#pragma once

#include <memory>
#include <string>

namespace ynet
{
	class ConnectionImpl;
	class ServerBackend;

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const std::string& host, uint16_t port);
	std::unique_ptr<ServerBackend> create_tcp_server(uint16_t port);
}
