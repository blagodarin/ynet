#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "tcp_backend.h"

namespace ynet
{
	class TcpClient: public Client
	{
	public:

		TcpClient(ClientCallbacks& callbacks, const std::string& host, int port);
		~TcpClient() override;

		// Client
		std::string host() const override;
		int port() const override;
		bool send(const void* data, size_t size) override;

		void close();

	private:

		void run();

	private:
	
		ClientCallbacks& _callbacks;
		const std::string _host;
		const int _port;
		const std::string _port_string;
		const int _reconnect_timeout;
		std::thread _thread;
		std::mutex _mutex;
		TcpBackend::Socket _socket;
		bool _shutting_down;
		bool _closing;
		std::condition_variable _closing_event;
		std::array<unsigned char, 48 * 1024> _buffer;
	};
}
