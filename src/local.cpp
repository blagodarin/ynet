#include "local.h"

#include <cassert>
#include <cstddef>
#include <map>
#include <vector>

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "backend.h"
#include "socket.h"

namespace ynet
{
	// All values are arbitrary.
	const size_t LocalBufferSize = 64 * 1024;
	const int LocalMaxPendingConnections = 16;

	namespace
	{
		std::pair<::sockaddr_un, size_t> make_local_sockaddr(uint16_t port)
		{
			::sockaddr_un sockaddr;
			::memset(&sockaddr, 0, sizeof sockaddr);
			sockaddr.sun_family = AF_UNIX;
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

	class LocalServer : public ServerBackend
	{
	public:

		LocalServer(Socket&& socket, const ::sockaddr_storage& sockaddr)
			: _socket(std::move(socket))
			, _sockaddr(sockaddr)
		{
		}

		~LocalServer() override = default;

		void poll(ServerHandlers& handlers) override
		{
			const auto make_pollfd = [](int socket)
			{
				::pollfd pollfd;
				::memset(&pollfd, 0, sizeof pollfd);
				pollfd.fd = socket;
				pollfd.events = POLLIN;
				return pollfd;
			};

			std::map<int, std::shared_ptr<ConnectionImpl>> connections;
			std::vector<uint8_t> receive_buffer(LocalBufferSize);

			const auto accept = [this, &connections, &handlers]()
			{
				Socket peer = ::accept(_socket.get(), nullptr, nullptr);
				if (!peer)
				{
					if (errno == ECONNABORTED)
						return true;
					if (errno == EINVAL)
						return false;
					throw std::system_error(errno, std::generic_category());
				}
				const auto peer_socket = peer.get();
				const std::shared_ptr<ConnectionImpl> connection(new SocketConnection(_sockaddr, std::move(peer), ConnectionSide::Server, LocalBufferSize));
				handlers.on_connected(connection);
				connections.emplace(peer_socket, connection);
				return true;
			};

			for (bool stopping = false; !stopping || !connections.empty(); )
			{
				std::vector<::pollfd> pollfds;
				pollfds.reserve(connections.size() + 1);
				for (const auto& connection : connections)
					pollfds.emplace_back(make_pollfd(connection.first));
				if (!stopping)
					pollfds.emplace_back(make_pollfd(_socket.get()));
				auto count = ::poll(pollfds.data(), pollfds.size(), -1);
				assert(count > 0);
				bool do_accept = false;
				bool do_stop = false;
				if (!stopping)
				{
					const auto revents = pollfds.back().revents;
					pollfds.pop_back();
					if (revents)
					{
						--count;
						if (revents == POLLIN)
							do_accept = true;
						else
							do_stop = true;
					}
				}
				for (const auto& pollfd : pollfds)
				{
					if (!pollfd.revents)
						continue;
					const auto i = connections.find(pollfd.fd);
					assert(i != connections.end());
					bool disconnected = pollfd.revents & (POLLHUP | POLLERR | POLLNVAL);
					if (pollfd.revents & POLLIN)
						handlers.on_received(i->second, receive_buffer.data(), receive_buffer.size(), disconnected);
					if (disconnected)
					{
						handlers.on_disconnected(i->second);
						connections.erase(i);
					}
				}
				if (do_accept && !accept())
					do_stop = true;
				if (do_stop)
				{
					stopping = true;
					for (const auto& connection : connections)
						connection.second->close();
					// TODO: Consider aborting the connection here instead of gracefully closing it.
					// The current implementation hangs if the client is constantly sending us data
					// and doesn't check whether the server has gracefully closed the connection.
				}
			}

			assert(connections.empty());
		}

		void shutdown() override
		{
			::shutdown(_socket.get(), SHUT_RD);
		}

	private:

		const Socket _socket;
		const ::sockaddr_storage _sockaddr;
	};

	std::unique_ptr<ConnectionImpl> create_local_connection(uint16_t port)
	{
		const auto& sockaddr = make_local_sockaddr(port);
		Socket socket = ::socket(sockaddr.first.sun_family, SOCK_STREAM, 0);
		if (!socket)
			return {};
		if (::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr.first), sockaddr.second) == -1)
			return {};
		return std::unique_ptr<ConnectionImpl>(new SocketConnection(make_sockaddr("127.0.0.1", port), std::move(socket), ConnectionSide::Client, LocalBufferSize));
		// TODO: Specify the correct loopback address (IPv4 or IPv6, depending on the server).
	}

	std::unique_ptr<ServerBackend> create_local_server(uint16_t port, bool ipv6_only)
	{
		const auto& sockaddr = make_local_sockaddr(port);
		Socket socket = ::socket(sockaddr.first.sun_family, SOCK_STREAM, 0);
		if (!socket)
			throw std::system_error(errno, std::generic_category());
		if (::bind(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr.first), sockaddr.second) == -1)
			return {};
		if (::listen(socket.get(), LocalMaxPendingConnections) == -1)
			return {};
		return std::unique_ptr<ServerBackend>(new LocalServer(std::move(socket), make_sockaddr(ipv6_only ? "::1" : "127.0.0.1", port)));
	}
}
