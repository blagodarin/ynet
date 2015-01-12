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
		std::string address() const override;
		void disconnect() override;
		std::string host() const override;
		std::string name() const override;
		int port() const override;
		bool send(const void* data, size_t size) override;

	private:

		void run();

	private:
	
		ClientCallbacks& _callbacks;
		const std::string _host;
		const int _port;
		const std::string _port_string;
		const int _reconnect_timeout;
		std::thread _thread;
		mutable std::mutex _mutex;
		TcpBackend::Socket _socket;
		std::string _address;
		std::string _name;
		bool _disconnecting;
		bool _stopping;
		std::condition_variable _stop_event;
	};
}
