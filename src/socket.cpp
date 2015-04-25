#include "socket.h"

#include <cassert>
#include <cstring>

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

	SocketConnection::SocketConnection(const Address& address, Socket&& socket, Side side, size_t receive_buffer_size)
		: ConnectionImpl(address)
		, _socket(std::move(socket))
		, _side(side)
		, _receive_buffer_size(receive_buffer_size)
	{
	}

	void SocketConnection::abort()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_aborted)
		{
			_aborted = true;
			::shutdown(_socket.get(), _closed ? SHUT_RD : SHUT_RDWR);
			_closed = true;
		}
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

	bool SocketConnection::send(const void* data, size_t size)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_closed)
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
					_closed = true;
					_aborted = true;
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
}
