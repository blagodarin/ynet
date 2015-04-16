#pragma once

#include <condition_variable>
#include <thread>

#include <ynet.h>
#include "address.h"
#include "backend.h"

namespace ynet
{
	class ServerImpl : public Server
	{
	public:

		ServerImpl(Callbacks& callbacks, uint16_t port, const Options& options);
		~ServerImpl() override;

		std::string address() const override;
		uint16_t port() const override;

	private:

		void run();
		void run_local();

	private:

		Callbacks& _callbacks;
		ServerHandlers _handlers;
		const Options _options;
		const int _relisten_timeout;
		const ::sockaddr_storage _sockaddr;
		const Address _address;
		std::mutex _mutex;
		ServerBackend* _backend = nullptr;
		bool _stopping = false;
		std::condition_variable _stop_event;
		std::thread _thread;
		ServerBackend* _local_backend = nullptr;
		std::condition_variable _local_server_started;
		bool _start_local_polling = false;
		std::condition_variable _start_local_polling_condition;
	};
}
