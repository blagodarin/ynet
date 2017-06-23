#pragma once

#include <memory>
#include <string>

namespace ynet
{
	std::unique_ptr<class ConnectionImpl> create_tcp_connection(const std::string& host, std::uint16_t port);
	std::unique_ptr<class ServerBackend> create_tcp_server(std::uint16_t port);
}
