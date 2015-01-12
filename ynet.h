#pragma once

#include <memory>
#include <string>

namespace ynet
{
	class Client;

	//!
	//! \note All callbacks are called from the client thread.
	class ClientCallbacks
	{
	public:

		virtual ~ClientCallbacks() = default;

		//!
		virtual void on_started(const Client& client);

		//!
		virtual void on_connected(Client& client) = 0;

		//!
		virtual void on_received(Client& client, const void* data, size_t size) = 0;

		//!
		virtual void on_disconnected(const Client& client) = 0;

		//!
		virtual void on_failed_to_connect(const Client& client);

		//!
		virtual void on_stopped(const Client& client);
	};

	//!
	class Client
	{
	public:

		//!
		static std::unique_ptr<Client> create(ClientCallbacks& callbacks, const std::string& host, int port);

		virtual ~Client() = default;

		//! Get the address of the remote party.
		//! \return Remote party address or empty string if there is no connection.
		virtual std::string address() const = 0;

		//! Disconnect from the remote party.
		//! \note Does nothing if there is no connection.
		virtual void disconnect() = 0;

		//! Get the remote host.
		//! \return Host as specified in Client::create, or an empty string if it is a server client.
		virtual std::string host() const = 0;

		//!
		virtual std::string name() const = 0;

		//! Get the remote port.
		//! \return Remote port.
		virtual int port() const = 0;

		//! Send a message to the remote party synchronously.
		//! \return \c true if the message was sent.
		virtual bool send(const void* data, size_t size) = 0;
	};

	class Server;

	//!
	//! \note All callbacks are called from the server thread.
	class ServerCallbacks
	{
	public:

		virtual ~ServerCallbacks() = default;

		//!
		virtual void on_started(const Server& server);

		//!
		virtual void on_connected(const Server& server, Client& client) = 0;

		//!
		virtual void on_received(const Server& server, Client& client, const void* data, size_t size) = 0;

		//!
		virtual void on_disconnected(const Server& server, const Client& client) = 0;

		//!
		virtual void on_stopped(const Server& server);
	};

	//!
	class Server
	{
	public:

		//!
		static std::unique_ptr<Server> create(ServerCallbacks& callbacks, int port);

		virtual ~Server() = default;

		//!
		virtual std::string address() const = 0;

		//!
		virtual std::string name() const = 0;

		//!
		virtual int port() const = 0;
	};
}
