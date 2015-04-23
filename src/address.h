#pragma once

#include <string>

#include <sys/socket.h>

namespace ynet
{
	class Address
	{
	public:

		enum class Family
		{
			IPv4,
			IPv6,
		};

		enum class Special
		{
			Any,
			Loopback,
		};

		explicit Address(const ::sockaddr_storage& sockaddr);
		Address(Family family, Special special, uint16_t port);

		Family family() const { return _family; }
		std::string ip() const;
		uint16_t port() const { return _port; }
		::sockaddr_storage sockaddr() const { return _sockaddr; }

	private:

		::sockaddr_storage _sockaddr;
		Family _family;
		uint16_t _port;
	};
}
