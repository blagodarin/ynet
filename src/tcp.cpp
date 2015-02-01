#include "tcp.h"

#include <cassert>
#include <cstring>
#include <map>

#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

#include "connection.h"

namespace ynet
{
	const size_t TcpBufferSize = 48 * 1024;

	class TcpSocket
	{
	public:

		TcpSocket()
			: _socket(-1)
		{
		}

		TcpSocket(int socket)
			: _socket(socket)
		{
		}

		TcpSocket(TcpSocket&& socket)
			: _socket(socket._socket)
		{
			socket._socket = -1;
		}

		~TcpSocket()
		{
			reset();
		}

		explicit operator bool() const
		{
			return _socket != -1;
		}

		int get() const
		{
			return _socket;
		}

		int release()
		{
			const auto socket = _socket;
			_socket = -1;
			return socket;
		}

		void reset()
		{
			if (_socket != -1)
			{
				::close(_socket);
				_socket = -1;
			}
		}

		TcpSocket(const TcpSocket&) = delete;
		TcpSocket& operator=(const TcpSocket&) = delete;
		TcpSocket& operator=(TcpSocket&&) = delete;

	private:

		int _socket;
	};

	class TcpConnection: public ConnectionImpl
	{
	public:

		TcpConnection(const ::sockaddr_storage& sockaddr, TcpSocket&& socket)
			: ConnectionImpl(sockaddr)
			, _socket(std::move(socket))
			, _closed(false)
		{
		}

		~TcpConnection() override
		{
			::close(_socket.get());
		}

		void close() override
		{
			std::lock_guard<std::mutex> lock(_mutex);
			if (!_closed)
			{
				_closed = true;
				::shutdown(_socket.get(), SHUT_WR);
			}
		}

		bool send(const void* data, size_t size) override
		{
			std::lock_guard<std::mutex> lock(_mutex);
			if (_closed)
				return false;
			const auto sent_size = ::send(_socket.get(), data, size, MSG_NOSIGNAL);
			if (sent_size == -1)
			{
				switch (errno)
				{
				case ECONNRESET:
					_closed = true;
					return false;
				default:
					throw std::system_error(errno, std::generic_category());
				}
			}
			else if (static_cast<size_t>(sent_size) != size)
				throw std::logic_error("size == " + std::to_string(size) + ", sent_size == " + std::to_string(sent_size));
			return true;
		}

		size_t receive(void* data, size_t size, bool* disconnected) override
		{
			const auto received_size = ::recv(_socket.get(), data, size, 0);
			if (received_size == -1)
			{
				switch (errno)
				{
				case EAGAIN:
			#if EWOULDBLOCK != EAGAIN
				case EWOULDBLOCK:
			#endif
					return 0;
				case ECONNRESET:
					if (disconnected)
						*disconnected = true;
					return 0;
				default:
					throw std::system_error(errno, std::generic_category());
				}
			}
			else if (received_size == 0)
			{
				if (disconnected)
					*disconnected = true;
				return 0;
			}
			else
				return received_size;
		}

	private:

		std::mutex _mutex;
		TcpSocket _socket;
		bool _closed;
	};

	TcpClient::TcpClient(Callbacks& callbacks, const std::string& host, uint16_t port, Trigger& trigger)
		: ClientImpl(callbacks, host, port, trigger)
	{
	}

	TcpClient::~TcpClient()
	{
		stop();
	}

	std::unique_ptr<ConnectionImpl> TcpClient::connect(const ::sockaddr_storage& sockaddr)
	{
		TcpSocket socket = ::socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (!socket)
			return {};
		if (::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sizeof sockaddr) == -1)
			return {};
		return std::unique_ptr<ConnectionImpl>(new TcpConnection(sockaddr, std::move(socket)));
	}

	size_t TcpClient::receive_buffer_size() const
	{
		return TcpBufferSize;
	}

	struct TcpServer::Private
	{
		std::mutex _mutex;
		int _socket = -1;
		bool _shutting_down = false;
	};

	TcpServer::TcpServer(Callbacks& callbacks, uint16_t port, Trigger& trigger)
		: ServerImpl(callbacks, port, trigger)
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
		TcpSocket socket = ::socket(sockaddr.ss_family, SOCK_STREAM, IPPROTO_TCP);
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
			TcpSocket peer = ::accept4(socket, reinterpret_cast<::sockaddr*>(&sockaddr), &sockaddr_size, SOCK_NONBLOCK);
			if (!peer)
			{
				if (errno == ECONNABORTED)
					return;
				throw std::system_error(errno, std::generic_category());
			}
			const auto peer_socket = peer.get();
			const std::shared_ptr<ConnectionImpl> connection(new TcpConnection(sockaddr, std::move(peer)));
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
}
