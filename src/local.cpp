#include "local.h"

#include <cassert>
#include <cstddef>

#include <sys/socket.h>
#include <sys/un.h>

#include "socket.h"

namespace ynet
{
	const char LocalAddress[] = "127.0.0.1";

	// All values are arbitrary.
	const size_t LocalBufferSize = 64 * 1024;
	const int LocalMaxPendingConnections = 16;

	namespace
	{
		std::pair<::sockaddr_un, size_t> make_local_sockaddr(uint16_t port)
		{
			::sockaddr_un sockaddr = {AF_UNIX};
			char* cursor = sockaddr.sun_path;
			*cursor++ = '\0'; // Linux-specific abstact socket.
			*cursor++ = 'y';
			*cursor++ = 'n';
			*cursor++ = 'e';
			*cursor++ = 't';
			*cursor++ = '.';
			::sprintf(cursor, "%d", port);
			const size_t sockaddr_size = offsetof(sockaddr_un, sun_path) + (cursor - sockaddr.sun_path) + ::strlen(cursor) + 1;
			return std::make_pair(sockaddr, sockaddr_size);
		}
	}

	class LocalServer : public SocketServer
	{
	public:

		LocalServer(Socket&& socket): SocketServer(std::move(socket), LocalBufferSize) {}
		~LocalServer() override = default;

		std::shared_ptr<SocketConnection> accept(int socket, bool& shutdown) override
		{
			const auto peer = ::accept(socket, nullptr, nullptr);
			if (peer != -1)
				return std::make_shared<SocketConnection>(LocalAddress, Socket(peer), SocketConnection::Side::Server, LocalBufferSize);
			switch (errno)
			{
			case ECONNABORTED:
				return {};
			case EINVAL:
				shutdown = true;
				return {};
			default:
				throw std::system_error(errno, std::generic_category());
			}
		};
	};

	std::unique_ptr<ConnectionImpl> create_local_connection(uint16_t port)
	{
		const auto& sockaddr = make_local_sockaddr(port);
		Socket socket(sockaddr.first.sun_family, SOCK_STREAM, 0);
		if (::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr.first), sockaddr.second) == -1)
			return {};
		return std::make_unique<SocketConnection>(LocalAddress, std::move(socket), SocketConnection::Side::Client, LocalBufferSize);
	}

	std::unique_ptr<ServerBackend> create_local_server(uint16_t port)
	{
		const auto& sockaddr = make_local_sockaddr(port);
		Socket socket(sockaddr.first.sun_family, SOCK_STREAM, 0);
		if (::bind(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr.first), sockaddr.second) == -1)
			return {};
		if (::listen(socket.get(), LocalMaxPendingConnections) == -1)
			return {};
		return std::make_unique<LocalServer>(std::move(socket));
	}
}
