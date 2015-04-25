#pragma once

#include <condition_variable>
#include <thread>

#include "address.h"
#include "backend.h"

namespace ynet
{
	class ServerImpl : public Server
	{
	public:

		ServerImpl(Callbacks&, uint16_t port, const Options& options);
		~ServerImpl() override;

		std::string address() const override { return _address.ip(); }
		uint16_t port() const override { return _address.port(); }

	private:

		void run();
		void run_local();

	private:

		Callbacks& _callbacks;
		ServerHandlers _handlers;
		const Options _options;
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
