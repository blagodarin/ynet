#pragma once

#include <mutex>

#include "backend.h"
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

		enum class State
		{
			Open,
			Closing,
			Closed,
		};

		SocketConnection(std::string&& address, Socket&& socket, Side side, size_t receive_buffer_size);
		~SocketConnection() override = default;

		void abort() override;
		bool send(const void* data, size_t size) override;
		void shutdown() override;

		size_t receive(void* data, size_t size, bool* disconnected) override;
		size_t receive_buffer_size() const override { return _receive_buffer_size; }

		int socket() const { return _socket.get(); }

	private:
		std::mutex _mutex;
		const Socket _socket;
		const Side _side;
		const size_t _receive_buffer_size;
		State _state = State::Open;
	};

	class SocketServer : public ServerBackend
	{
	public:
		SocketServer(Socket&& socket, size_t buffer_size) : _socket(std::move(socket)), _buffer_size(buffer_size) {}
		~SocketServer() override = default;

		void run(Callbacks& callbacks) final;
		void shutdown(int milliseconds) final;

		virtual std::shared_ptr<SocketConnection> accept(int socket, bool& shutdown) = 0;

	private:
		const Socket _socket;
		const size_t _buffer_size;
	};
}
