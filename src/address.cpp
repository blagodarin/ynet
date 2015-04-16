#include "address.h"

#include <cstring>
#include <system_error>

#include <arpa/inet.h>
#include <netdb.h>

namespace ynet
{
	namespace
	{
		bool is_local(const ::sockaddr_in& sockaddr)
		{
			union
			{
				uint8_t u8[4];
			} ip;
			static_assert(sizeof ip == sizeof sockaddr.sin_addr, "");
			::memcpy(&ip, &sockaddr.sin_addr, sizeof sockaddr.sin_addr);
			if (ip.u8[0] == 127)
				return true; // 127.0.0.0/8
			// TODO: Check other addresses of the local machine.
			return false;
		}

		bool is_local(const ::sockaddr_in6& sockaddr)
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
					return true; // ::1
				if (ip.u16[5] == 0xFFFF && ip.u8[12] == 127)
					return true; // ::ffff::127.0.0.0/104
			}
			// TODO: Check other addresses of the local machine.
			return false;
		}
	}

	std::string make_address(const ::sockaddr_storage& sockaddr)
	{
		if (sockaddr.ss_family == AF_INET)
		{
			const auto& sockaddr_in = *reinterpret_cast<const ::sockaddr_in*>(&sockaddr);
			char buffer[INET_ADDRSTRLEN];
			if (!::inet_ntop(AF_INET, &sockaddr_in.sin_addr, buffer, INET_ADDRSTRLEN))
				throw std::system_error(errno, std::generic_category());
			return buffer;
		}
		else if (sockaddr.ss_family == AF_INET6)
		{
			const auto& sockaddr_in6 = *reinterpret_cast<const ::sockaddr_in6*>(&sockaddr);
			char buffer[INET6_ADDRSTRLEN];
			if (!::inet_ntop(AF_INET6, &sockaddr_in6.sin6_addr, buffer, INET6_ADDRSTRLEN))
				throw std::system_error(errno, std::generic_category());
			return buffer;
		}
		else
			throw std::logic_error("Only IPv4/IPv6 addresses are supported");
	}

	::sockaddr_storage make_sockaddr(const std::string& address, uint16_t port)
	{
		::sockaddr_storage sockaddr_storage;
		::memset(&sockaddr_storage, 0, sizeof sockaddr_storage);

		auto& sockaddr_in = *reinterpret_cast<::sockaddr_in*>(&sockaddr_storage);
		if (::inet_pton(AF_INET, address.c_str(), &sockaddr_in.sin_addr) == 1)
		{
			sockaddr_in.sin_family = AF_INET;
			sockaddr_in.sin_port = ::htons(port);
			return sockaddr_storage;
		}

		auto& sockaddr_in6 = *reinterpret_cast<::sockaddr_in6*>(&sockaddr_storage);
		if (::inet_pton(AF_INET6, address.c_str(), &sockaddr_in6.sin6_addr) == 1)
		{
			sockaddr_in6.sin6_family = AF_INET6;
			sockaddr_in6.sin6_port = ::htons(port);
			return sockaddr_storage;
		}

		throw std::logic_error("Only IPv4/IPv6 addresses are supported");
		// It may also be that the system doesn't support IPv6 (or both IPv4 and IPv6).
	}

	Address::Address(const ::sockaddr_storage& sockaddr)
	{
		// TODO: Merge this and make_address.
		if (sockaddr.ss_family == AF_INET)
		{
			const auto& sockaddr_in = *reinterpret_cast<const ::sockaddr_in*>(&sockaddr);
			char buffer[INET_ADDRSTRLEN];
			if (!::inet_ntop(AF_INET, &sockaddr_in.sin_addr, buffer, INET_ADDRSTRLEN))
				throw std::system_error(errno, std::generic_category());
			_address = buffer;
			_port = ::ntohs(sockaddr_in.sin_port);
		}
		else if (sockaddr.ss_family == AF_INET6)
		{
			const auto& sockaddr_in6 = *reinterpret_cast<const ::sockaddr_in6*>(&sockaddr);
			char buffer[INET6_ADDRSTRLEN];
			if (!::inet_ntop(AF_INET6, &sockaddr_in6.sin6_addr, buffer, INET6_ADDRSTRLEN))
				throw std::system_error(errno, std::generic_category());
			_address = buffer;
			_port = ::ntohs(sockaddr_in6.sin6_port);
		}
		else
			throw std::logic_error("Only IPv4/IPv6 addresses are supported");
	}

	Resolved resolve(const std::string& host, uint16_t port)
	{
		struct Resolver
		{
			::addrinfo* addrinfos = nullptr;

			Resolver(const std::string& host)
			{
				::addrinfo hints;
				::memset(&hints, 0, sizeof hints);
				hints.ai_family = AF_UNSPEC;
				// The soket type field isn't required for address resolution,
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
		Resolved resolved;
		for (const auto* addrinfo = resolver.addrinfos; addrinfo; addrinfo = addrinfo->ai_next)
		{
			::sockaddr_storage sockaddr;
			if (addrinfo->ai_family == AF_INET)
			{
				::sockaddr_in& sockaddr_in = *reinterpret_cast<::sockaddr_in*>(&sockaddr);
				::memcpy(&sockaddr_in, addrinfo->ai_addr, sizeof sockaddr_in);
				sockaddr_in.sin_port = ::htons(port);
				resolved.local = is_local(sockaddr_in);
			}
			else if (addrinfo->ai_family != AF_INET6)
			{
				::sockaddr_in6& sockaddr_in6 = *reinterpret_cast<::sockaddr_in6*>(&sockaddr);
				::memcpy(&sockaddr_in6, addrinfo->ai_addr, sizeof sockaddr_in6);
				sockaddr_in6.sin6_port = ::htons(port);
				resolved.local = is_local(sockaddr_in6);
			}
			else
				continue;
			resolved.addresses.emplace_back(sockaddr);
		}
		return resolved;
	}
}
