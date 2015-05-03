#pragma once

#include <condition_variable>
#include <thread>

#include <ynet.h>

namespace ynet
{
	class ServerBackend;

	class ServerImpl : public Server
	{
	public:

		ServerImpl(Callbacks&, const std::function<std::unique_ptr<ServerBackend>()>& factory);
		~ServerImpl() override;

		void set_shutdown_timeout(int milliseconds) override { _shutdown_timeout = milliseconds; }

	private:

		void run();

	private:

		Callbacks& _callbacks;
		const std::function<std::unique_ptr<ServerBackend>()> _factory;
		std::mutex _mutex;
		ServerBackend* _backend = nullptr;
		bool _stopping = false;
		std::condition_variable _stop_event;
		int _shutdown_timeout = 0;
		std::thread _thread;
	};
}
