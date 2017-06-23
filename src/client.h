#pragma once

#include <condition_variable>
#include <thread>

#include <ynet.h>

namespace ynet
{
	class ConnectionImpl;

	class ClientImpl : public Client
	{
	public:
		ClientImpl(Callbacks&, const Options&, const std::function<std::unique_ptr<ConnectionImpl>()>& factory);
		~ClientImpl() override;

	private:
		void run();

	private:
		Callbacks& _callbacks;
		const Options _options;
		const std::function<std::unique_ptr<ConnectionImpl>()> _factory;
		std::mutex _mutex;
		ConnectionImpl* _connection = nullptr;
		bool _stopping = false;
		std::condition_variable _stop_event;
		std::condition_variable _disconnect_event;
		std::thread _thread;
	};
}
