#include "local.h"

#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "backend.h"
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

		LocalServer(Socket&& socket): _socket(std::move(socket)) {}
		~LocalServer() override = default;

		void run(ServerBackend::Callbacks& callbacks) override
		{
			std::unordered_map<int, std::shared_ptr<ConnectionImpl>> connections;
			std::vector<uint8_t> receive_buffer(LocalBufferSize);

			const auto accept = [this, &connections, &callbacks]()
			{
				const auto peer = ::accept(_socket.get(), nullptr, nullptr);
				if (peer == -1)
				{
					if (errno == ECONNABORTED)
						return true;
					if (errno == EINVAL)
						return false;
					throw std::system_error(errno, std::generic_category());
				}
				const std::shared_ptr<ConnectionImpl> connection(new SocketConnection(LocalAddress, Socket(peer), SocketConnection::Side::Server, LocalBufferSize));
				callbacks.on_connected(connection);
				connections.emplace(peer, connection);
				return true;
			};

			for (bool stopping = false; !stopping || !connections.empty(); )
			{
				std::vector<::pollfd> pollfds;
				pollfds.reserve(connections.size() + 1);
				for (const auto& connection : connections)
					pollfds.emplace_back(::pollfd{connection.first, POLLIN});
				if (!stopping)
					pollfds.emplace_back(::pollfd{_socket.get(), POLLIN});
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
						callbacks.on_received(i->second, receive_buffer.data(), receive_buffer.size(), disconnected);
					if (disconnected)
					{
						callbacks.on_disconnected(i->second);
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
