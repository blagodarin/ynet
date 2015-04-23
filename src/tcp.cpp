#include "tcp.h"

#include <cassert>
#include <cstring>
#include <map>
#include <vector>

#include <netinet/in.h>
#include <poll.h>

#include "backend.h"
#include "socket.h"

namespace ynet
{
	// All values are arbitrary.
	const size_t TcpBufferSize = 64 * 1024;
	const int TcpMaxPendingConnections = 16;

	class TcpServer : public ServerBackend
	{
	public:

		TcpServer(Socket&& socket): _socket(std::move(socket)) {}
		~TcpServer() override = default;

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
			std::vector<uint8_t> receive_buffer(TcpBufferSize);

			const auto accept = [&connections, &handlers](int socket)
			{
				::sockaddr_storage sockaddr;
				auto sockaddr_size = sizeof sockaddr;
				Socket peer = ::accept(socket, reinterpret_cast<::sockaddr*>(&sockaddr), &sockaddr_size);
				if (!peer)
				{
					if (errno == ECONNABORTED)
						return;
					throw std::system_error(errno, std::generic_category());
				}
				const auto peer_socket = peer.get();
				const std::shared_ptr<ConnectionImpl> connection(new SocketConnection(Address(sockaddr), std::move(peer), ConnectionSide::Server, TcpBufferSize));
				handlers.on_connected(connection);
				connections.emplace(peer_socket, connection);
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
				if (do_accept)
					accept(_socket.get());
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

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const Address& address)
	{
		const auto& sockaddr = address.sockaddr();
		Socket socket = ::socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (!socket)
			return {};
		if (::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr) == -1)
			return {};
		return std::make_unique<SocketConnection>(address, std::move(socket), ConnectionSide::Client, TcpBufferSize);
	}

	std::unique_ptr<ServerBackend> create_tcp_server(const Address& address)
	{
		const auto& sockaddr = address.sockaddr();
		Socket socket = ::socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (!socket)
			throw std::system_error(errno, std::generic_category());
		if (::bind(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr) == -1)
			return {};
		if (::listen(socket.get(), TcpMaxPendingConnections) == -1)
			return {};
		return std::make_unique<TcpServer>(std::move(socket));
	}
}
