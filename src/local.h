#pragma once

#include <memory>
#include <string>

namespace ynet
{
	std::unique_ptr<class ConnectionImpl> create_local_connection(const std::string& name);
	std::unique_ptr<class ServerBackend> create_local_server(const std::string& name);
}
