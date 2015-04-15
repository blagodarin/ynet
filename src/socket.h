#pragma once

#include <mutex>

#include "connection.h"

namespace ynet
{
	class Socket
	{
	public:

		Socket(int socket)
			: _socket(socket)
		{
		}

		Socket(Socket&& socket)
			: _socket(socket._socket)
		{
			socket._socket = -1;
		}

		~Socket();

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

		Socket() = default;
		Socket(const Socket&) = delete;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = delete;

	private:

		int _socket = -1;
	};

	class SocketConnection : public ConnectionImpl
	{
	public:

		enum
		{
			NonblockingRecv = 1 << 0,
		};

		SocketConnection(std::string&& address, Socket&& socket, unsigned flags, size_t receive_buffer_size);
		~SocketConnection() override = default;

		void abort() override;
		void close() override;
		bool exhausted() const override;
		bool send(const void* data, size_t size) override;
		size_t receive(void* data, size_t size, bool* disconnected) override;
		size_t receive_buffer_size() const override;

	private:

		std::mutex _mutex;
		Socket _socket;
		const unsigned _flags;
		const size_t _receive_buffer_size;
		bool _closed = false;
		bool _aborted = false;
	};
}
