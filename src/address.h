#pragma once

#include <string>
#include <vector>

#include <sys/socket.h>

namespace ynet
{
	::sockaddr_storage make_sockaddr(const std::string& address, uint16_t port);

	struct Address
	{
		std::string _address;
		uint16_t _port;
		std::string _name;

		explicit Address(const ::sockaddr_storage& sockaddr);
		Address(const std::string& address, int port): Address(make_sockaddr(address, port)) {}

		Address() = default;
		Address(const Address&) = delete;
		Address(Address&&) = default;
		~Address() = default;
		Address& operator=(const Address&) = delete;
		Address& operator=(Address&&) = default;
	};

	struct Resolved
	{
		std::vector<::sockaddr_storage> addresses;
		bool local = false;
	};

	Resolved resolve(const std::string& host, uint16_t port);
}
