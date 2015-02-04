#pragma once

#include <condition_variable>
#include <thread>

#include <ynet.h>
#include "address.h"

namespace ynet
{
	class ConnectionImpl;

	class ServerImpl: public Server
	{
	public:

		ServerImpl(Callbacks& callbacks, uint16_t port);
		~ServerImpl() override;

		std::string address() const override;
		std::string name() const override;
		uint16_t port() const override;

		virtual void start();
		void stop();

		void on_connected(const std::shared_ptr<Connection>& connection);
		void on_received(const std::shared_ptr<Connection>& connection, void* buffer, size_t buffer_size, bool& disconnected);
		void on_disconnected(const std::shared_ptr<Connection>& connection);

	protected:

		virtual bool listen(const ::sockaddr_storage& sockaddr) = 0;
		virtual void poll() = 0;
		virtual void shutdown() = 0;

	private:

		void run();

	private:

		Callbacks& _callbacks;
		const ::sockaddr_storage _sockaddr;
		const Address _address;
		const int _relisten_timeout;
		std::mutex _mutex;
		bool _stopping;
		std::condition_variable _stop_event;
		std::thread _thread;
	};
}
