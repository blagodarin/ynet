#include "tcp.h"

#include <cassert>
#include <cstring>
#include <map>

#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

#include "socket.h"

namespace ynet
{
	const size_t TcpBufferSize = 64 * 1024; // Arbitrary value.

	struct TcpServer::Private
	{
		std::mutex _mutex;
		int _socket = -1;
		bool _shutting_down = false;
	};

	TcpServer::TcpServer(Callbacks& callbacks, uint16_t port, const Options& options)
		: ServerImpl(callbacks, port, options)
		, _private(new Private())
	{
	}

	TcpServer::~TcpServer()
	{
		stop();
		if (_private->_socket != -1)
			::close(_private->_socket);
	}

	bool TcpServer::listen(const ::sockaddr_storage& sockaddr)
	{
		Socket socket = ::socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (!socket)
			throw std::system_error(errno, std::generic_category());
		if (::bind(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr) == -1)
			return false;
		if (::listen(socket.get(), 16) == -1)
			return false;
		std::lock_guard<std::mutex> lock(_private->_mutex);
		if (_private->_shutting_down)
			return false;
		_private->_socket = socket.release();
		return true;
	}

	void TcpServer::poll()
	{
		assert(_private->_socket != -1);

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

		const auto accept = [this, &connections](int socket)
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
			const std::shared_ptr<ConnectionImpl> connection(
				new SocketConnection(sockaddr, std::move(peer), SocketConnection::NonblockingRecv, TcpBufferSize));
			on_connected(connection);
			connections.emplace(peer_socket, connection);
		};

		for (bool stopping = false; !stopping || !connections.empty(); )
		{
			std::vector<::pollfd> pollfds;
			pollfds.reserve(connections.size() + 1);
			for (const auto& connection : connections)
				pollfds.emplace_back(make_pollfd(connection.first));
			if (!stopping)
				pollfds.emplace_back(make_pollfd(_private->_socket));
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
					on_received(i->second, receive_buffer.data(), receive_buffer.size(), disconnected);
				if (disconnected)
				{
					on_disconnected(i->second);
					connections.erase(i);
				}
			}
			if (do_accept)
				accept(_private->_socket);
			if (do_stop)
			{
				stopping = true;
				for (const auto& connection : connections)
					connection.second->close();
				// TODO: Consider aborting the connection here instead of gracefully closing it.
				// The current implementation hangs if the client is constantly sending us data
				// and doesn't check for the server to become exhausted.
			}
		}

		assert(connections.empty());
	}

	void TcpServer::shutdown()
	{
		std::lock_guard<std::mutex> lock(_private->_mutex);
		if (_private->_socket == -1)
			return;
		::shutdown(_private->_socket, SHUT_RD);
		_private->_shutting_down = true;
	}

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const ::sockaddr_storage& sockaddr)
	{
		Socket socket = ::socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (!socket)
			return {};
		if (::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr) == -1)
			return {};
		return std::unique_ptr<ConnectionImpl>(new SocketConnection(sockaddr, std::move(socket), 0, TcpBufferSize));
	}
}
