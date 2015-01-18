#pragma once

#include <condition_variable>
#include <thread>

#include <ynet.h>

namespace ynet
{
	class ConnectionImpl;
	class Trigger;

	class ClientImpl: public Client
	{
	public:

		ClientImpl(Callbacks& callbacks, const std::string& host, uint16_t port, Trigger& trigger);
		~ClientImpl() override;

		std::string host() const override;
		uint16_t port() const override;

		void stop();

	protected:

		virtual std::shared_ptr<Connection> connect(const std::string& host, const std::string& port) = 0;
		virtual size_t receive_buffer_size() const = 0;

	private:

		void run();

	private:
	
		Callbacks& _callbacks;
		const std::string _host;
		const uint16_t _port;
		const std::string _port_string;
		const int _reconnect_timeout;
		std::mutex _mutex;
		ConnectionImpl* _connection;
		bool _stopping;
		std::condition_variable _stop_event;
		std::thread _thread;
	};
}