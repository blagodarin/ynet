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
		ServerImpl(Callbacks&, const Options&, const std::function<std::unique_ptr<ServerBackend>()>& factory);
		~ServerImpl() override;

	private:
		void run();

	private:
		Callbacks& _callbacks;
		const Options _options;
		const std::function<std::unique_ptr<ServerBackend>()> _factory;
		std::mutex _mutex;
		ServerBackend* _backend = nullptr;
		bool _stopping = false;
		std::condition_variable _stop_event;
		std::thread _thread;
	};
}
