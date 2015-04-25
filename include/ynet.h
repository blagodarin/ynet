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
			// The default implementation does nothing.
			virtual void on_started();

			// Called when the client has connected to the server.
			virtual void on_connected(const std::shared_ptr<Connection>&) = 0;

			// Called when the client has received a message.
			virtual void on_received(const std::shared_ptr<Connection>&, const void* data, size_t size) = 0;

			// Called when the client has been disconnected from the server.
			// 'reconnect_timeout' should be set to a nonnegative value
			// to try to reconnect in the specified number of milliseconds.
			virtual void on_disconnected(const std::shared_ptr<Connection>&, int& reconnect_timeout) = 0;

			// Called when a connection attempt fails.
			// 'reconnect_timeout' should be set to a nonnegative value
			// to try to reconnect in the specified number of milliseconds.
			virtual void on_failed_to_connect(int& reconnect_timeout) = 0;

			// Called when any network activity has finished.
			// The default implementation does nothing.
			virtual void on_stopped();
		};

		//
		struct Options
		{
			bool optimized_loopback; //

			Options();
		};

		// Creates a client.
		static std::unique_ptr<Client> create(Callbacks&, const std::string& host, uint16_t port, const Options& options = {});

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

		// All callbacks are called from a server thread.
		// There may be multiple server threads, but all callbacks for any
		// single connection are guaranteed to be called from the same thread.
		class Callbacks
		{
		public:

			virtual ~Callbacks() = default;

			// Called if the server has failed to start.
			// 'restart_timeout' should be set to a nonnegative value
			// to try to restart in the specified number of milliseconds.
			virtual void on_failed_to_start(int& restart_timeout) = 0;

			// Called when the server has started, but before any clients connect.
			virtual void on_started();

			// Called when a client has connected to the server.
			virtual void on_connected(const std::shared_ptr<Connection>&) = 0;

			// Called when the server has received a message from a client.
			virtual void on_received(const std::shared_ptr<Connection>&, const void* data, size_t size) = 0;

			// Called when a client has been disconnected from the server.
			virtual void on_disconnected(const std::shared_ptr<Connection>&) = 0;

			// Called when any network activity has finished.
			// The default implementation does nothing.
			virtual void on_stopped();
		};

		//
		struct Options
		{
			bool optimized_loopback; //

			Options();
		};

		// Creates a server.
		static std::unique_ptr<Server> create(Callbacks&, uint16_t port, const Options& options = Options());

		virtual ~Server() = default;

		// Returns the address the server is bound to.
		virtual std::string address() const = 0;

		// Returns the port the server is bound to.
		virtual uint16_t port() const = 0;
	};
}
