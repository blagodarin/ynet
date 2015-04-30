#include "tcp.h"

#include <cassert>
#include <cstring>
#include <map>
#include <vector>

#include <netinet/in.h>
#include <poll.h>

#include "backend.h"
#include "resolution.h"
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

		void run(ServerBackend::Callbacks& callbacks) override
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

			const auto accept = [&connections, &callbacks](int socket)
			{
				::sockaddr_storage sockaddr = {};
				auto sockaddr_size = sizeof sockaddr;
				const auto peer = ::accept(socket, reinterpret_cast<::sockaddr*>(&sockaddr), &sockaddr_size);
				if (peer == -1)
				{
					if (errno == ECONNABORTED)
						return;
					throw std::system_error(errno, std::generic_category());
				}
				const std::shared_ptr<ConnectionImpl> connection(new SocketConnection(Address(sockaddr), Socket(peer), SocketConnection::Side::Server, TcpBufferSize));
				callbacks.on_connected(connection);
				connections.emplace(peer, connection);
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
						callbacks.on_received(i->second, receive_buffer.data(), receive_buffer.size(), disconnected);
					if (disconnected)
					{
						callbacks.on_disconnected(i->second);
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

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const std::string& host, uint16_t port)
	{
		for (const auto& sockaddr : resolve(host, port))
		{
			Socket socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
			if (-1 != ::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr))
				return std::make_unique<SocketConnection>(Address(sockaddr), std::move(socket), SocketConnection::Side::Client, TcpBufferSize);
		}
		return {};
	}

	std::unique_ptr<ServerBackend> create_tcp_server(uint16_t port)
	{
		// TODO: Add (optional) IPv6 support.
		const Address address(Address::Family::IPv4, Address::Special::Any, port);
		const auto& sockaddr = address.sockaddr();
		Socket socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (::bind(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr) == -1)
			return {};
		if (::listen(socket.get(), TcpMaxPendingConnections) == -1)
			return {};
		return std::make_unique<TcpServer>(std::move(socket));
	}
}
