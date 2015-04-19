#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace ynet
{
	// Network connection.
	class Connection
	{
	public:

		virtual ~Connection() = default;

		//
		virtual void abort() = 0;

		// Returns the address of the remote party.
		virtual std::string address() const = 0;

		// Gracefully closes the connection.
		virtual void close() = 0;

		// Synchronously send a message to the remote party.
		// Returns true if the message has been sent.
		virtual bool send(const void* data, size_t size) = 0;
	};

	// Network client.
	class Client
	{
	public:

		// All callbacks are called from the client thread.
		class Callbacks
		{
		public:

			virtual ~Callbacks() = default;

			// Called before any network activity starts.
			virtual void on_started(const Client& client);

			// Called when the client has connected to the server.
			virtual void on_connected(const Client& client, const std::shared_ptr<Connection>& connection) = 0;

			// Called when the client has received a message.
			virtual void on_received(const Client& client, const std::shared_ptr<Connection>& connection, const void* data, size_t size) = 0;

			// Called when the client has been disconnected from the server.
			// 'reconnect_timeout' should be set to a nonnegative value for the client to reconnect in the specified number of milliseconds.
			virtual void on_disconnected(const Client& client, const std::shared_ptr<Connection>& connection, int& reconnect_timeout) = 0;

			// Called when a connection attempt fails.
			// 'initial' is true for the first connection attempt of the client.
			// 'reconnect_timeout' should be set to a nonnegative value for the client to reconnect in the specified number of milliseconds.
			virtual void on_failed_to_connect(const Client& client, bool initial, int& reconnect_timeout) = 0;

			// Called after any network activity has finished.
			virtual void on_stopped(const Client& client);
		};

		//
		struct Options
		{
			bool optimized_loopback; //

			Options();
		};

		//
		static std::unique_ptr<Client> create(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options = {});

		virtual ~Client() = default;

		// Returns the host for the client to connect to (as was specified in create).
		virtual std::string host() const = 0;

		// Returns the port for the client to connect to.
		virtual uint16_t port() const = 0;
	};

	// Network server.
	class Server
	{
	public:

		// All callbacks are called from the server thread.
		class Callbacks
		{
		public:

			virtual ~Callbacks() = default;

			//
			virtual void on_failed_to_start(const Server& server, bool initial, int& restart_timeout) = 0;

			//
			virtual void on_started(const Server& server);

			//
			virtual void on_connected(const Server& server, const std::shared_ptr<Connection>& connection) = 0;

			//
			virtual void on_received(const Server& server, const std::shared_ptr<Connection>& connection, const void* data, size_t size) = 0;

			//
			virtual void on_disconnected(const Server& server, const std::shared_ptr<Connection>& connection) = 0;

			//
			virtual void on_stopped(const Server& server);
		};

		//
		struct Options
		{
			bool optimized_loopback; //

			Options();
		};

		//
		static std::unique_ptr<Server> create(Callbacks& callbacks, uint16_t port, const Options& options = Options());

		virtual ~Server() = default;

		// Returns the address the server is bound to.
		virtual std::string address() const = 0;

		// Returns the port the server is bound to.
		virtual uint16_t port() const = 0;
	};
}
