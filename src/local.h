#pragma once

#include <memory>
#include <string>

namespace ynet
{
	class ConnectionImpl;
	class ServerBackend;

	std::unique_ptr<ConnectionImpl> create_local_connection(const std::string& name);
	std::unique_ptr<ServerBackend> create_local_server(const std::string& name);
}
