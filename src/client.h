#pragma once

#include <condition_variable>
#include <thread>

#include <ynet.h>

struct sockaddr_storage;

namespace ynet
{
	class ConnectionImpl;

	class ClientImpl: public Client
	{
	public:

		ClientImpl(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options);
		~ClientImpl() override;

		std::string host() const override;
		uint16_t port() const override;

	private:

		void run();

	private:
	
		Callbacks& _callbacks;
		const std::string _host;
		const uint16_t _port;
		const Options _options;
		std::mutex _mutex;
		ConnectionImpl* _connection;
		bool _stopping;
		std::condition_variable _stop_event;
		std::thread _thread;
	};
}
