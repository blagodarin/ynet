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
		std::pair<::sockaddr_un, size_t> make_local_sockaddr(const std::string& name)
		{
			if (name.empty())
				throw std::logic_error("Name \"" + name + "\" is invalid");
			::sockaddr_un sockaddr = {AF_UNIX};
			// The significant part of sockaddr.sun_path is "<name>\0" for ordinary sockets
			// and "\0<name>" for Linux-specific abstract sockets; its size is always the same.
			// Names without any sigils are enough library specific to be limited
			// by one less than the actual maximum length possible.
			if (name.size() > sizeof sockaddr.sun_path - 1)
				throw std::logic_error("Name \"" + name + "\" is too long");
			if (name[0] == '/')
			{
				::memcpy(sockaddr.sun_path, name.data(), name.size());
				return std::make_pair(sockaddr, offsetof(sockaddr_un, sun_path) + name.size() + 1);
			}
#ifdef __linux__
			else if (name[0] == '@')
			{
				::memcpy(sockaddr.sun_path + 1, name.data() + 1, name.size() - 1);
				return std::make_pair(sockaddr, offsetof(sockaddr_un, sun_path) + name.size());
			}
			else
			{
				::memcpy(sockaddr.sun_path + 1, name.data(), name.size());
				return std::make_pair(sockaddr, offsetof(sockaddr_un, sun_path) + name.size() + 1);
			}
#else
			else
				throw std::logic_error("Name \"" + name + "\" is invalid");
#endif
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

	std::unique_ptr<ConnectionImpl> create_local_connection(const std::string& name)
	{
		const auto& sockaddr = make_local_sockaddr(name);
		Socket socket(sockaddr.first.sun_family, SOCK_STREAM, 0);
		if (::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr.first), sockaddr.second) == -1)
			return {};
		return std::make_unique<SocketConnection>(LocalAddress, std::move(socket), SocketConnection::Side::Client, LocalBufferSize);
	}

	std::unique_ptr<ServerBackend> create_local_server(const std::string& name)
	{
		const auto& sockaddr = make_local_sockaddr(name);
		Socket socket(sockaddr.first.sun_family, SOCK_STREAM, 0);
		if (::bind(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr.first), sockaddr.second) == -1)
			return {};
		if (::listen(socket.get(), LocalMaxPendingConnections) == -1)
			return {};
		return std::make_unique<LocalServer>(std::move(socket));
	}
}
