#pragma once

#include <mutex>

#include "tcp_backend.h"

namespace ynet
{
	class TcpConnection: public Connection
	{
	public:

		TcpConnection(TcpBackend::Socket socket, std::string&& address, int port, std::string&& name);
		TcpConnection(const TcpConnection&) = delete;
		TcpConnection& operator=(const TcpConnection&) = delete;

		std::string address() const override;
		void close() override;
		std::string name() const override;
		int port() const override;
		bool send(const void* data, size_t size) override;

		explicit operator bool() const
		{
			return _socket != TcpBackend::InvalidSocket;
		}

	private:

		std::mutex _socket_mutex;
		TcpBackend::Socket _socket;
		const std::string _address;
		const int _port;
		const std::string _name;
	};
}
