#pragma once

#include <string>
#include <vector>

#include <sys/socket.h>

namespace ynet
{
	::sockaddr_storage any_ipv4(uint16_t port);
	::sockaddr_storage any_ipv6(uint16_t port);
	::sockaddr_storage loopback_ipv4(uint16_t port);
	::sockaddr_storage loopback_ipv6(uint16_t port);

	struct Address
	{
		std::string _address;
		uint16_t _port = 0;

		explicit Address(const ::sockaddr_storage& sockaddr);

		Address() = delete;
		Address(const Address&) = delete;
		Address(Address&&) = delete;
		Address& operator=(const Address&) = delete;
		Address& operator=(Address&&) = delete;
	};

	struct Resolved
	{
		std::vector<::sockaddr_storage> addresses;
		bool local = false;
	};

	Resolved resolve(const std::string& host, uint16_t port);
}
