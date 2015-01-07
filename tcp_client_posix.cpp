#include "tcp_client_posix.h"

#include "client.h"

#include <cassert>
#include <cstring>

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

namespace ynet
{
	namespace
	{
		bool convert(const ::sockaddr_storage& sockaddr, std::string& address, int& port)
		{
			const auto address_family = sockaddr.ss_family;
			if (address_family == AF_INET)
			{
				char buffer[INET_ADDRSTRLEN];
				if (::inet_ntop(address_family, &reinterpret_cast<const ::sockaddr_in*>(&sockaddr)->sin_addr, buffer, sizeof buffer))
				{
					address = buffer;
					port = ::ntohs(reinterpret_cast<const ::sockaddr_in*>(&sockaddr)->sin_port);
					return true;
				}
			}
			else if (address_family == AF_INET6)
			{
				char buffer[INET6_ADDRSTRLEN];
				if (::inet_ntop(address_family, &reinterpret_cast<const ::sockaddr_in6*>(&sockaddr)->sin6_addr, buffer, sizeof buffer))
				{
					address = buffer;
					port = ::ntohs(reinterpret_cast<const ::sockaddr_in6*>(&sockaddr)->sin6_port);
					return true;
				}
			}
			return false;
		}
	}

	namespace TcpClientBackend
	{
		void close(Socket socket)
		{
			::close(socket);
		}

		Socket connect(const std::string& host, const std::string& port, ClientConnection& connection)
		{
			::addrinfo* addrinfos = nullptr;
			{
				::addrinfo hints;
				::memset(&hints, 0, sizeof hints);
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				if (::getaddrinfo(host.c_str(), port.c_str(), &hints, &addrinfos) != 0)
					return InvalidSocket;
			}
			Socket socket = InvalidSocket;
			for (const auto* addrinfo = addrinfos; addrinfo; addrinfo = addrinfo->ai_next)
			{
				socket = ::socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
				if (socket == InvalidSocket)
					continue;
				if (convert(*reinterpret_cast<const ::sockaddr_storage*>(addrinfo->ai_addr), connection.remote_address, connection.remote_port)
					&& ::connect(socket, addrinfo->ai_addr, addrinfo->ai_addrlen) != -1)
				{
					::sockaddr_storage sockaddr;
					size_t sockaddr_size = sizeof sockaddr;
					if (::getsockname(socket, reinterpret_cast<::sockaddr*>(&sockaddr), &sockaddr_size) != -1
						&& convert(sockaddr, connection.local_address, connection.local_port))
					{
						// TODO: Keep-alive.
						break;
					}
				}
				::close(socket);
			}
			::freeaddrinfo(addrinfos);
			return socket;
		}

		size_t recv(Socket socket, void* data, size_t size)
		{
			const auto received_size = ::recv(socket, data, size, 0);
			if (received_size == 0 || received_size == -1)
				return 0;
			return received_size;
		}

		bool send(Socket socket, const void* data, size_t size)
		{
			if (::send(socket, data, size, 0) == static_cast<ssize_t>(size))
				return true;
			// TODO: Disconnect?
			return false;
		}

		void shutdown(Socket socket)
		{
			::shutdown(socket, SHUT_WR);
		}
	}
}
