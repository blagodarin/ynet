#include "address.h"

#include <cstring>
#include <system_error>

#include <arpa/inet.h>

namespace ynet
{
	Address::Address(const ::sockaddr_storage& sockaddr)
		: _sockaddr(sockaddr)
	{
		switch (sockaddr.ss_family)
		{
		case AF_INET:
			_family = Family::IPv4;
			_port = ::ntohs(reinterpret_cast<const ::sockaddr_in&>(sockaddr).sin_port);
			break;
		case AF_INET6:
			_family = Family::IPv6;
			_port = ::ntohs(reinterpret_cast<const ::sockaddr_in6&>(sockaddr).sin6_port);
			break;
		default:
			throw std::logic_error("Only IPv4/IPv6 addresses are supported");
		}
	}

	Address::Address(Family family, Special special, uint16_t port)
		: _family(family)
		, _port(port)
	{
		::memset(&_sockaddr, 0, sizeof _sockaddr);
		switch (_family)
		{
		case Family::IPv4:
			{
				auto& sockaddr_in = reinterpret_cast<::sockaddr_in&>(_sockaddr);
				sockaddr_in.sin_family = AF_INET;
				switch (special)
				{
				case Special::Any:
					sockaddr_in.sin_addr.s_addr = ::htonl(INADDR_ANY);
					break;
				case Special::Loopback:
					sockaddr_in.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
					break;
				default:
					throw std::logic_error("Bad special address");
				}
				sockaddr_in.sin_port = ::htons(_port);
			}
			break;
		case Family::IPv6:
			{
				auto& sockaddr_in6 = reinterpret_cast<::sockaddr_in6&>(_sockaddr);
				sockaddr_in6.sin6_family = AF_INET6;
				switch (special)
				{
				case Special::Any:
					sockaddr_in6.sin6_addr = IN6ADDR_ANY_INIT;
					break;
				case Special::Loopback:
					sockaddr_in6.sin6_addr = IN6ADDR_LOOPBACK_INIT;
					break;
				default:
					throw std::logic_error("Bad special address");
				}
				sockaddr_in6.sin6_port = ::htons(_port);
			}
			break;
		default:
			throw std::logic_error("Bad address family");
		}
	}

	std::string Address::ip() const
	{
		switch (_family)
		{
		case Family::IPv4:
			{
				char buffer[INET_ADDRSTRLEN];
				if (!::inet_ntop(AF_INET, &reinterpret_cast<const ::sockaddr_in&>(_sockaddr).sin_addr, buffer, INET_ADDRSTRLEN))
					throw std::system_error(errno, std::generic_category());
				return buffer;
			}
		case Family::IPv6:
			{
				char buffer[INET6_ADDRSTRLEN];
				if (!::inet_ntop(AF_INET6, &reinterpret_cast<const ::sockaddr_in6&>(_sockaddr).sin6_addr, buffer, INET6_ADDRSTRLEN))
					throw std::system_error(errno, std::generic_category());
				return buffer;
			}
		default:
			throw std::logic_error("Bad address family");
		}
	}
}
