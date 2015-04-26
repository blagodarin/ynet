#include "resolution.h"

#include <climits>
#include <cstring>
#include <system_error>

#include <netdb.h>
#include <unistd.h>

#include "address.h"

namespace ynet
{
	namespace
	{
		bool is_loopback(const ::sockaddr_in& sockaddr)
		{
			union
			{
				uint8_t u8[4];
			} ip;
			static_assert(sizeof ip == sizeof sockaddr.sin_addr, "");
			::memcpy(&ip, &sockaddr.sin_addr, sizeof sockaddr.sin_addr);
			if (ip.u8[0] == 127)
				return true; // 127.0.0.0/8
			return false;
		}

		bool is_loopback(const ::sockaddr_in6& sockaddr)
		{
			union
			{
				uint8_t u8[16];
				uint16_t u16[8];
				uint32_t u32[4];
				uint64_t u64[2];
			} ip;
			static_assert(sizeof ip == sizeof sockaddr.sin6_addr, "");
			::memcpy(&ip, &sockaddr.sin6_addr, sizeof sockaddr.sin6_addr);
			if (ip.u64[0] == 0 && ip.u16[4] == 0)
			{
				if (ip.u16[5] == 0 && ip.u16[6] == 0 && ip.u8[14] == 0 && ip.u8[15] == 1)
					return true; // ::1/128
				if (ip.u16[5] == 0xFFFF && ip.u8[12] == 127)
					return true; // ::ffff:127.0.0.0/104
			}
			return false;
		}

		std::string host_name()
		{
			char name[HOST_NAME_MAX + 1];
			if (::gethostname(name, HOST_NAME_MAX + 1) == -1)
				throw std::system_error(errno, std::generic_category());
			name[HOST_NAME_MAX] = '\0'; // An implementation may silently truncate a longer name.
			return name;
		}
	}

	Resolution::Resolution(const std::string& host, uint16_t port)
	{
		struct Resolver
		{
			::addrinfo* addrinfos = nullptr;

			Resolver(const std::string& host)
			{
				::addrinfo hints;
				::memset(&hints, 0, sizeof hints);
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

		const Resolver local(host_name());
		const Resolver resolved(host);
		for (const auto* resolved_addrinfo = resolved.addrinfos; resolved_addrinfo; resolved_addrinfo = resolved_addrinfo->ai_next)
		{
			const ::sockaddr* resolved_sockaddr = resolved_addrinfo->ai_addr;
			::sockaddr_storage sockaddr;
			if (resolved_sockaddr->sa_family == AF_INET)
			{
				::sockaddr_in& sockaddr_in = reinterpret_cast<::sockaddr_in&>(sockaddr);
				::memcpy(&sockaddr_in, resolved_sockaddr, sizeof sockaddr_in);
				sockaddr_in.sin_port = ::htons(port);
				if (!_local)
				{
					if (is_loopback(sockaddr_in))
						_local = true;
					else
					{
						for (const auto* local_addrinfo = local.addrinfos; local_addrinfo; local_addrinfo = local_addrinfo->ai_next)
						{
							if (local_addrinfo->ai_addr->sa_family != AF_INET)
								continue;
							if (!::memcmp(&reinterpret_cast<const ::sockaddr_in*>(local_addrinfo->ai_addr)->sin_addr,
								&sockaddr_in.sin_addr, sizeof sockaddr_in.sin_addr))
							{
								_local = true;
								break;
							}
						}
					}
				}
			}
			else if (resolved_sockaddr->sa_family == AF_INET6)
			{
				::sockaddr_in6& sockaddr_in6 = reinterpret_cast<::sockaddr_in6&>(sockaddr);
				::memcpy(&sockaddr_in6, resolved_sockaddr, sizeof sockaddr_in6);
				sockaddr_in6.sin6_port = ::htons(port);
				if (!_local)
				{
					if (is_loopback(sockaddr_in6))
						_local = true;
					else
					{
						for (const auto* local_addrinfo = local.addrinfos; local_addrinfo; local_addrinfo = local_addrinfo->ai_next)
						{
							if (local_addrinfo->ai_addr->sa_family != AF_INET6)
								continue;
							if (!::memcmp(&reinterpret_cast<const ::sockaddr_in6*>(local_addrinfo->ai_addr)->sin6_addr,
								&sockaddr_in6.sin6_addr, sizeof sockaddr_in6.sin6_addr))
							{
								_local = true;
								break;
							}
						}
					}
				}
			}
			else
				continue;
			_addresses.emplace_back(sockaddr);
		}
	}
}
