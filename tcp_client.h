#pragma once

#include <condition_variable>
#include <thread>

#include "tcp_backend.h"

namespace ynet
{
	class TcpConnection;

	class TcpClient: public Client
	{
	public:

		TcpClient(Callbacks& callbacks, const std::string& host, int port);
		~TcpClient() override;

		std::string host() const override;
		int port() const override;

	private:

		void run();

	private:
	
		Callbacks& _callbacks;
		const std::string _host;
		const int _port;
		const std::string _port_string;
		const int _reconnect_timeout;
		std::thread _thread;
		std::mutex _mutex;
		TcpConnection* _connection;
		bool _stopping;
		std::condition_variable _stop_event;
	};
}
