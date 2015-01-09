#include "tcp_backend.h"

#include <cassert>
#include <cstring>

#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
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

	namespace TcpBackend
	{
		void close(Socket socket)
		{
			::close(socket);
		}

		Socket connect(const std::string& host, const std::string& port, std::string& address)
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
				int remote_port = -1;
				if (convert(*reinterpret_cast<const ::sockaddr_storage*>(addrinfo->ai_addr), address, remote_port)
					&& ::connect(socket, addrinfo->ai_addr, addrinfo->ai_addrlen) != -1)
				{
					// TODO: Keep-alive.
					break;
				}
				close(socket);
				socket = InvalidSocket; // In case it's the last addrinfo.
			}
			::freeaddrinfo(addrinfos);
			return socket;
		}

		Socket listen(int port, std::string& address)
		{
			::sockaddr_storage sockaddr;
			::memset(&sockaddr, 0, sizeof sockaddr);
			sockaddr.ss_family = AF_INET;
			reinterpret_cast<::sockaddr_in*>(&sockaddr)->sin_port = ::htons(port);
			if (convert(sockaddr, address, port))
			{
				const Socket socket = ::socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
				if (socket != InvalidSocket)
				{
					if (::bind(socket, reinterpret_cast<::sockaddr*>(&sockaddr), sizeof sockaddr) == 0
						&& ::listen(socket, 16) == 0)
						return socket;
					close(socket);
				}
			}
			return InvalidSocket;
		}

		size_t recv(Socket socket, void* data, size_t size, bool* disconnected)
		{
			const auto received_size = ::recv(socket, data, size, 0);
			if (received_size == 0)
			{
				if (disconnected)
					*disconnected = true;
				return 0;
			}
			else if (received_size == -1)
			{
				if (disconnected)
					*disconnected = errno != EAGAIN && errno != EWOULDBLOCK;
				return 0;
			}
			else
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

		void shutdown_server(Socket socket)
		{
			::shutdown(socket, SHUT_RD);
		}

		void Poller::run(Socket socket)
		{
			const auto make_pollfd = [](Socket socket)
			{
				::pollfd pollfd;
				::memset(&pollfd, 0, sizeof pollfd);
				pollfd.fd = socket;
				pollfd.events = POLLIN;
				return pollfd;
			};

			std::vector<Socket> peers;

			const auto accept = [this, &peers](Socket socket)
			{
				::sockaddr_storage sockaddr;
				size_t sockaddr_size = sizeof sockaddr;
				const Socket peer = ::accept4(socket, reinterpret_cast<::sockaddr*>(&sockaddr), &sockaddr_size, SOCK_NONBLOCK);
				if (peer == InvalidSocket)
					return;
				std::string address;
				int port = -1;
				if (!convert(sockaddr, address, port))
				{
					close(peer);
					return;
				}
				peers.emplace_back(peer);
				_callbacks.on_connected(peer, std::move(address), port);
			};

			while (socket != InvalidSocket || !peers.empty())
			{
				std::vector<::pollfd> pollfds;
				pollfds.reserve(peers.size() + 1);
				for (const auto peer : peers)
					pollfds.emplace_back(make_pollfd(peer));
				if (socket != InvalidSocket)
					pollfds.emplace_back(make_pollfd(socket));
				auto count = ::poll(pollfds.data(), pollfds.size(), -1);
				assert(count > 0);
				bool do_accept = false;
				bool do_stop = false;
				if (socket != InvalidSocket)
				{
					const auto revents = pollfds.back().revents;
					pollfds.pop_back();
					if (revents)
					{
						if (revents == POLLIN)
							do_accept = true;
						else
							do_stop = true;
						--count;
					}
				}
				for (const auto& pollfd : pollfds)
				{
					if (!pollfd.revents)
						continue;
					bool disconnected = pollfd.revents & (POLLHUP | POLLERR | POLLNVAL);
					if (pollfd.revents & POLLIN)
						_callbacks.on_received(pollfd.fd, disconnected);
					if (disconnected)
					{
						for (auto i = peers.begin(); i != peers.end(); ++i)
						{
							if (*i == pollfd.fd)
							{
								peers.erase(i);
								break;
							}
						}
						close(pollfd.fd);
						_callbacks.on_disconnected(pollfd.fd);
					}
				}
				if (do_accept)
					accept(socket);
				if (do_stop)
				{
					socket = InvalidSocket;
					for (const auto peer : peers)
						shutdown(peer);
				}
			}
		}
	}
}
