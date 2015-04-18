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
