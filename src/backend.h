#pragma once

#include <ynet.h>

namespace ynet
{
	class ServerHandlers
	{
	public:

		ServerHandlers(Server::Callbacks& callbacks): _callbacks(callbacks) {}

		void on_connected(const std::shared_ptr<Connection>& connection) { _callbacks.on_connected(connection); }
		void on_received(const std::shared_ptr<Connection>&, void* buffer, size_t buffer_size, bool& disconnected);
		void on_disconnected(const std::shared_ptr<Connection>& connection) { _callbacks.on_disconnected(connection); }

	private:

		Server::Callbacks& _callbacks;
	};

	class ServerBackend
	{
	public:

		virtual ~ServerBackend() = default;

		virtual void run(ServerHandlers&) = 0;
		virtual void shutdown() = 0;
	};
}
