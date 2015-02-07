#pragma once

#include <condition_variable>
#include <thread>

#include <ynet.h>
#include "address.h"
#include "backend.h"

namespace ynet
{
	class ServerImpl: public Server
	{
	public:

		ServerImpl(Callbacks& callbacks, uint16_t port, const Options& options);
		~ServerImpl() override;

		std::string address() const override;
		std::string name() const override;
		uint16_t port() const override;

	private:

		void run();

	private:

		Callbacks& _callbacks;
		ServerHandlers _handlers;
		const Options _options;
		const int _relisten_timeout;
		const ::sockaddr_storage _sockaddr;
		const Address _address;
		std::mutex _mutex;
		bool _stopping = false;
		ServerBackend* _backend = nullptr;
		std::condition_variable _stop_event;
		std::thread _thread;
	};
}
