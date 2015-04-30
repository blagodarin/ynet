#pragma once

#include <string>
#include <vector>

struct sockaddr_storage;

namespace ynet
{
	std::vector<::sockaddr_storage> resolve(const std::string& host, uint16_t port);
	std::string to_string(const ::sockaddr_storage&);
}
