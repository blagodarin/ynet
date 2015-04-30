#pragma once

#include <mutex>

#include "connection.h"

namespace ynet
{
	class Socket
	{
	public:

		explicit Socket(int socket);
		Socket(int domain, int type, int protocol);
		Socket(Socket&& socket);
		~Socket();

		int get() const { return _socket; }

		Socket() = delete;
		Socket(const Socket&) = delete;
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&&) = delete;

	private:

		int _socket;
	};

	class SocketConnection : public ConnectionImpl
	{
	public:

		enum class Side
		{
			Client,
			Server,
		};

		SocketConnection(std::string&& address, Socket&& socket, Side side, size_t receive_buffer_size);
		~SocketConnection() override = default;

		void abort() override;
		void close() override;
		bool send(const void* data, size_t size) override;
		size_t receive(void* data, size_t size, bool* disconnected) override;
		size_t receive_buffer_size() const override { return _receive_buffer_size; }

	private:

		std::mutex _mutex;
		const Socket _socket;
		const Side _side;
		const size_t _receive_buffer_size;
		bool _closed = false;
		bool _aborted = false;
	};
}
