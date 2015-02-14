#include "socket.h"

#include <cstring>

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace ynet
{
	Socket::~Socket()
	{
		if (_socket != -1)
			::close(_socket);
	}

	SocketConnection::SocketConnection(std::string&& address, Socket&& socket, unsigned flags, size_t receive_buffer_size)
		: ConnectionImpl(std::move(address))
		, _socket(std::move(socket))
		, _flags(flags)
		, _receive_buffer_size(receive_buffer_size)
	{
	}

	void SocketConnection::close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_closed)
		{
			_closed = true;
			::shutdown(_socket.get(), SHUT_WR);
		}
	}

	bool SocketConnection::exhausted() const
	{
		::pollfd pollfd;
		::memset(&pollfd, 0, sizeof pollfd);
		pollfd.fd = _socket.get();
		pollfd.events = POLLRDHUP;
		if (::poll(&pollfd, 1, 0) < 0)
			throw std::system_error(errno, std::generic_category());
		return pollfd.revents;
	}

	bool SocketConnection::send(const void* data, size_t size)
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
		{
			// Neither POSIX nor 'man send' explicitly state this can't happen.
			throw std::logic_error("Blocking 'send' sent " + std::to_string(sent_size) + " bytes of " + std::to_string(size) + " requested");
		}
		return true;
	}

	size_t SocketConnection::receive(void* data, size_t size, bool* disconnected)
	{
		const auto received_size = ::recv(_socket.get(), data, size, _flags & NonblockingRecv ? MSG_DONTWAIT : 0);
		if (received_size == -1)
		{
			switch (errno)
			{
			case EAGAIN:
		#if EWOULDBLOCK != EAGAIN
			case EWOULDBLOCK:
		#endif
				if (!(_flags & NonblockingRecv))
				{
					// "...a receive timeout had been set and the timeout expired before data was received." (c) 'man recv'
					throw std::logic_error("Blocking 'recv' timed out");
				}
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

	size_t SocketConnection::receive_buffer_size() const
	{
		return _receive_buffer_size;
	}
}
