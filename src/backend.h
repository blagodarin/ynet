#pragma once

#include <ynet.h>

namespace ynet
{
	class ServerHandlers
	{
	public:

		ServerHandlers(Server& server, Server::Callbacks& callbacks): _server(server), _callbacks(callbacks) {}

		void on_connected(const std::shared_ptr<Connection>& connection);
		void on_received(const std::shared_ptr<Connection>& connection, void* buffer, size_t buffer_size, bool& disconnected);
		void on_disconnected(const std::shared_ptr<Connection>& connection);

	private:

		Server& _server;
		Server::Callbacks& _callbacks;
	};

	class ServerBackend
	{
	public:

		virtual ~ServerBackend() = default;

		virtual void poll(ServerHandlers& handlers) = 0;
		virtual void shutdown() = 0;
	};
}
