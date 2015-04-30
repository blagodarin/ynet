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

		ClientImpl(Callbacks&, const std::function<std::unique_ptr<ConnectionImpl>()>& factory);
		~ClientImpl() override;

		void set_disconnect_timeout(int milliseconds) { _disconnect_timeout = milliseconds; }

	private:

		void run();

	private:
	
		Callbacks& _callbacks;
		const std::function<std::unique_ptr<ConnectionImpl>()> _factory;
		std::mutex _mutex;
		ConnectionImpl* _connection = nullptr;
		bool _stopping = false;
		std::condition_variable _stop_event;
		int _disconnect_timeout = 0;
		std::condition_variable _disconnect_event;
		std::thread _thread;
	};
}
