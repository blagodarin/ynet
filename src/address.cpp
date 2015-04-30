#include "address.h"

#include <system_error>

#include <arpa/inet.h>
#include <netdb.h>

namespace ynet
{
	std::vector<::sockaddr_storage> resolve(const std::string& host, uint16_t port)
	{
		struct Resolver
		{
			::addrinfo* addrinfos = nullptr;

			Resolver(const std::string& host)
			{
				::addrinfo hints = {};
				hints.ai_family = AF_UNSPEC;
				// The socket type field isn't required for address resolution,
				// but prevents getaddrinfo returning multiple addresses with different socket types.
				hints.ai_socktype = SOCK_STREAM;
				::getaddrinfo(host.c_str(), nullptr, &hints, &addrinfos);
			}

			~Resolver()
			{
				if (addrinfos)
					::freeaddrinfo(addrinfos);
			}
		};

		const Resolver resolver(host);

		std::vector<::sockaddr_storage> addresses;
		for (const auto* addrinfo = resolver.addrinfos; addrinfo; addrinfo = addrinfo->ai_next)
		{
			::sockaddr_storage sockaddr = {};
			if (addrinfo->ai_addr->sa_family == AF_INET)
			{
				::sockaddr_in& sockaddr_in = reinterpret_cast<::sockaddr_in&>(sockaddr);
				sockaddr_in = *reinterpret_cast<const ::sockaddr_in*>(addrinfo->ai_addr);
				sockaddr_in.sin_port = ::htons(port);
			}
			else if (addrinfo->ai_addr->sa_family == AF_INET6)
			{
				::sockaddr_in6& sockaddr_in6 = reinterpret_cast<::sockaddr_in6&>(sockaddr);
				sockaddr_in6 = *reinterpret_cast<const ::sockaddr_in6*>(addrinfo->ai_addr);
				sockaddr_in6.sin6_port = ::htons(port);
			}
			else
				continue;
			addresses.emplace_back(sockaddr);
		}
		return addresses;
	}

	std::string to_string(const ::sockaddr_storage& sockaddr)
	{
		switch (sockaddr.ss_family)
		{
		case AF_INET:
			{
				char buffer[INET_ADDRSTRLEN];
				if (!::inet_ntop(AF_INET, &reinterpret_cast<const ::sockaddr_in&>(sockaddr).sin_addr, buffer, INET_ADDRSTRLEN))
					throw std::system_error(errno, std::generic_category());
				return buffer;
			}
		case AF_INET6:
			{
				char buffer[INET6_ADDRSTRLEN];
				if (!::inet_ntop(AF_INET6, &reinterpret_cast<const ::sockaddr_in6&>(sockaddr).sin6_addr, buffer, INET6_ADDRSTRLEN))
					throw std::system_error(errno, std::generic_category());
				return buffer;
			}
		default:
			throw std::logic_error("Only IPv4/IPv6 addresses are supported");
		}
	}
}
