#include "address.h"

#include <cstring>
#include <system_error>

#include <arpa/inet.h>

namespace ynet
{
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

		throw std::invalid_argument("'address' must be a valid IPv4/IPv6 address");
		// It may also be that the system doesn't support IPv6 (or both IPv4 and IPv6).
	}

	Address::Address(const ::sockaddr_storage& sockaddr)
	{
		const auto address_family = sockaddr.ss_family;
		if (address_family == AF_INET)
		{
			const auto& sockaddr_in = *reinterpret_cast<const ::sockaddr_in*>(&sockaddr);
			char buffer[INET_ADDRSTRLEN + 1 + 5]; // 'ipv4:port'.
			char* cursor = buffer;
			if (!::inet_ntop(AF_INET, &sockaddr_in.sin_addr, cursor, INET_ADDRSTRLEN))
				throw std::system_error(errno, std::generic_category());
			_address = cursor;
			_port = ::ntohs(sockaddr_in.sin_port);
			cursor += _address.size();
			*cursor++ = ':';
			::sprintf(cursor, "%d", _port);
			_name = buffer;
		}
		else if (address_family == AF_INET6)
		{
			const auto& sockaddr_in6 = *reinterpret_cast<const ::sockaddr_in6*>(&sockaddr);
			char buffer[1 + INET6_ADDRSTRLEN + 1 + 1 + 5]; // '[ipv6]:port'.
			char* cursor = buffer;
			*cursor++ = '[';
			if (::inet_ntop(AF_INET6, &sockaddr_in6.sin6_addr, cursor, INET6_ADDRSTRLEN))
				throw std::system_error(errno, std::generic_category());
			_address = cursor;
			_port = ::ntohs(sockaddr_in6.sin6_port);
			cursor += _address.size();
			*cursor++ = ']';
			*cursor++ = ':';
			::sprintf(cursor, "%d", _port);
			_name = buffer;
		}
		else
			throw std::system_error(EAFNOSUPPORT, std::generic_category());
	}
}
