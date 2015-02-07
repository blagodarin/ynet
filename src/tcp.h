#pragma once

#include <memory>

struct sockaddr_storage;

namespace ynet
{
	class ConnectionImpl;
	class ServerBackend;

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const ::sockaddr_storage& sockaddr);
	std::unique_ptr<ServerBackend> create_tcp_server(const ::sockaddr_storage& sockaddr);
}
