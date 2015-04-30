#include "resolution.h"

//#include <cstring>

#include <netdb.h>
//#include <unistd.h>

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
				//::memcpy(&sockaddr_in, addrinfo->ai_addr, sizeof sockaddr_in);
				sockaddr_in = *reinterpret_cast<const ::sockaddr_in*>(addrinfo->ai_addr);
				sockaddr_in.sin_port = ::htons(port);
			}
			else if (addrinfo->ai_addr->sa_family == AF_INET6)
			{
				::sockaddr_in6& sockaddr_in6 = reinterpret_cast<::sockaddr_in6&>(sockaddr);
				//::memcpy(&sockaddr_in6, addrinfo->ai_addr, sizeof sockaddr_in6);
				sockaddr_in6 = *reinterpret_cast<const ::sockaddr_in6*>(addrinfo->ai_addr);
				sockaddr_in6.sin6_port = ::htons(port);
			}
			else
				continue;
			addresses.emplace_back(sockaddr);
		}
		return addresses;
	}
}
