#include "socket.h"

#include <cassert>
#include <unordered_map>
#include <vector>

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace ynet
{
	Socket::Socket(int socket)
		: _socket(socket)
	{
		assert(_socket != -1);
	}

	Socket::Socket(int domain, int type, int protocol)
		: _socket(::socket(domain, type, protocol))
	{
		if (_socket == -1)
			throw std::system_error(errno, std::generic_category());
	}

	Socket::Socket(Socket&& socket)
		: _socket(socket._socket)
	{
		socket._socket = -1;
	}

	Socket::~Socket()
	{
		if (_socket != -1)
			::close(_socket);
	}

	SocketConnection::SocketConnection(std::string&& address, Socket&& socket, Side side, size_t receive_buffer_size)
		: ConnectionImpl(std::move(address))
		, _socket(std::move(socket))
		, _side(side)
		, _receive_buffer_size(receive_buffer_size)
	{
	}

	void SocketConnection::abort()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_state != State::Closed)
		{
			::shutdown(_socket.get(), _state == State::Closing ? SHUT_RD : SHUT_RDWR);
			_state = State::Closed;
		}
	}

	void SocketConnection::shutdown()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_state == State::Open)
		{
			_state = State::Closing;
			::shutdown(_socket.get(), SHUT_WR);
		}
	}

	bool SocketConnection::send(const void* data, size_t size)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_state != State::Open)
			return false;
		for (size_t offset = 0; offset < size; )
		{
			const auto sent_size = ::send(_socket.get(), static_cast<const uint8_t*>(data) + offset, size - offset, MSG_NOSIGNAL);
			if (sent_size == -1)
			{
				switch (errno)
				{
				case ECONNRESET:
				case EPIPE:
					_state = State::Closed;
					return false;
				default:
					throw std::system_error(errno, std::generic_category());
				}
			}
			else
			{
				// This should mean the connection was broken during a blocking send.
				// However, there is no guarantee that this has really happened,
				// so we should try to send the remaining part of the buffer.
				offset += static_cast<size_t>(sent_size);
			}
		}
		return true;
	}

	size_t SocketConnection::receive(void* data, size_t size, bool* disconnected)
	{
		assert(size > 0);
		const bool nonblocking = _side == Side::Server;
		const auto received_size = ::recv(_socket.get(), data, size, nonblocking ? MSG_DONTWAIT : 0);
		if (received_size == -1)
		{
			switch (errno)
			{
			case EAGAIN:
		#if EWOULDBLOCK != EAGAIN
			case EWOULDBLOCK:
		#endif
				if (!nonblocking)
				{
					// "...a receive timeout had been set and the timeout expired before data was received." (c) 'man recv'
					throw std::logic_error("Blocking 'recv' timed out");
				}
				return 0;
			case ECONNRESET:
			case EPIPE:
				if (disconnected)
					*disconnected = true;
				return 0;
			default:
				throw std::system_error(errno, std::generic_category());
			}
		}
		else if (received_size == 0)
		{
			// The remote peer has performed a graceful shutdown.
			if (disconnected)
				*disconnected = true;
			return 0;
		}
		else
			return received_size;
	}

	void SocketServer::run(Callbacks& callbacks)
	{
		std::vector<uint8_t> receive_buffer(_buffer_size);
		std::unordered_map<int, std::shared_ptr<SocketConnection>> connections;
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
			if (do_accept)
			{
				assert(!do_stop);
				const auto& connection = accept(_socket.get(), do_stop);
				if (connection)
				{
					callbacks.on_connected(connection);
					connections.emplace(connection->socket(), connection);
				}
			}
			if (do_stop)
			{
				stopping = true;
				for (const auto& connection : connections)
					connection.second->shutdown();
			}
		}
		assert(connections.empty());
	}

	void SocketServer::shutdown(int milliseconds)
	{
		::shutdown(_socket.get(), SHUT_RD);
		// TODO: Limit the time for the server to shut down.
		// The current implementation hangs if a client is constantly sending us data
		// and doesn't check whether the server has gracefully closed the connection,
		// or doesn't reply to a graceful shutdown request anything at all.
	}
}
