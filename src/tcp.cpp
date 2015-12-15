#include "tcp.h"

#include <cassert>

#include <netinet/in.h>

#include "address.h"
#include "socket.h"

namespace ynet
{
	// All values are arbitrary.
	const size_t TcpBufferSize = 64 * 1024;
	const int TcpMaxPendingConnections = 16;

	class TcpServer : public SocketServer
	{
	public:
		TcpServer(Socket&& socket) : SocketServer(std::move(socket), TcpBufferSize) {}
		~TcpServer() override = default;

		std::shared_ptr<SocketConnection> accept(int socket, bool& shutdown) override
		{
			::sockaddr_storage sockaddr = {};
			auto sockaddr_size = sizeof sockaddr;
			const auto peer = ::accept(socket, reinterpret_cast<::sockaddr*>(&sockaddr), &sockaddr_size);
			if (peer != -1)
				return std::make_shared<SocketConnection>(to_string(sockaddr), Socket(peer), SocketConnection::Side::Server, TcpBufferSize);
			switch (errno)
			{
			case ECONNABORTED:
				return {};
			default:
				throw std::system_error(errno, std::generic_category());
			}
		};
	};

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const std::string& host, uint16_t port)
	{
		for (const auto& sockaddr : resolve(host, port))
		{
			Socket socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
			if (-1 != ::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr))
				return std::make_unique<SocketConnection>(to_string(sockaddr), std::move(socket), SocketConnection::Side::Client, TcpBufferSize);
		}
		return {};
	}

	std::unique_ptr<ServerBackend> create_tcp_server(uint16_t port)
	{
		::sockaddr_storage sockaddr = {};
		// TODO: Add (optional) IPv6 support.
		sockaddr.ss_family = AF_INET;
		reinterpret_cast<::sockaddr_in&>(sockaddr).sin_port = ::htons(port);
		Socket socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (::bind(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr) == -1)
			return {};
		if (::listen(socket.get(), TcpMaxPendingConnections) == -1)
			return {};
		return std::make_unique<TcpServer>(std::move(socket));
	}
}
