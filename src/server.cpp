#include "server.h"

#include <cassert>

#include "backend.h"

namespace ynet
{
	ServerImpl::ServerImpl(Callbacks& callbacks, const Options& options, const std::function<std::unique_ptr<ServerBackend>()>& factory)
		: _callbacks{callbacks}
		, _options{options}
		, _factory{factory}
		, _thread{[this]{ run(); }}
	{
	}

	ServerImpl::~ServerImpl()
	{
		assert(_thread.joinable());
		assert(_thread.get_id() != std::this_thread::get_id());
		{
			std::lock_guard<std::mutex> lock{_mutex};
			_stopping = true;
			if (_backend)
			{
				_backend->shutdown(_options.shutdown_timeout);
				_backend = nullptr;
			}
		}
		_stop_event.notify_one();
		_thread.join();
	}

	void ServerImpl::run()
	{
		std::unique_ptr<ServerBackend> backend;
		for (;;)
		{
			backend = _factory();
			if (backend)
			{
				std::lock_guard<std::mutex> lock{_mutex};
				if (_stopping)
					return;
				_backend = backend.get();
				break;
			}
			int restart_timeout = -1;
			_callbacks.on_failed_to_start(restart_timeout);
			if (restart_timeout < 0)
				return;
			std::unique_lock<std::mutex> lock{_mutex};
			if (restart_timeout > 0)
			{
				if (_stop_event.wait_for(lock, std::chrono::milliseconds(restart_timeout), [this]{ return _stopping; }))
					return;
			}
			else if (_stopping)
				return;
		}
		_callbacks.on_started();
		ServerBackend::Callbacks backend_callbacks{_callbacks};
		backend->run(backend_callbacks);
	}
}
