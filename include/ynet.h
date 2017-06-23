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

		// Aborts the connection, interrupting all active IO operations, if any,
		// and preventing new ones from starting.
		virtual void abort() = 0;

		// Returns the peer IP address.
		virtual std::string address() const = 0;

		// Synchronously sends a block of data to the peer.
		// Returns true if the entire block was sent.
		virtual bool send(const void* data, size_t size) = 0;

		// Initiates a graceful shutdown.
		// The connection can't be used to send data after this function is called,
		// but data may still be received before the connection terminates.
		virtual void shutdown() = 0;
	};

	// Network client.
	class Client
	{
	public:

		// All callbacks are called from the client thread.
		struct Callbacks
		{
			virtual ~Callbacks() = default;

			// Called just before the network activity starts.
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

			// Called right after the network activity stops.
			// The default implementation does nothing.
			virtual void on_stopped();
		};

		// Client options.
		struct Options
		{
			// Number of milliseconds to wait for graceful disconnect during client destruction.
			// A negative value means infinite timeout. Zero means instant connection reset.
			int shutdown_timeout = 0;

			constexpr Options() noexcept {}
		};

		// Creates a local client.
		static std::unique_ptr<Client> create_local(Callbacks&, const std::string& name, const Options& = {});

		// Creates a TCP client.
		static std::unique_ptr<Client> create_tcp(Callbacks&, const std::string& host, uint16_t port, const Options& = {});

		virtual ~Client() = default;
	};

	// Network server.
	class Server
	{
	public:

		// All callbacks are called from the server thread.
		// No server functions may be called from the callbacks.
		struct Callbacks
		{
			virtual ~Callbacks() = default;

			// Called if the server has failed to start.
			// 'restart_timeout' should be set to a nonnegative value
			// to try to restart in the specified number of milliseconds.
			virtual void on_failed_to_start(int& restart_timeout) = 0;

			// Called when the server has started, but before any clients connect.
			// The default implementation does nothing.
			virtual void on_started();

			// Called when a client has connected to the server.
			virtual void on_connected(const std::shared_ptr<Connection>&) = 0;

			// Called when the server has received a message from a client.
			virtual void on_received(const std::shared_ptr<Connection>&, const void* data, size_t size) = 0;

			// Called when a client has been disconnected from the server.
			virtual void on_disconnected(const std::shared_ptr<Connection>&) = 0;
		};

		// Server options.
		struct Options
		{
			// Number of milliseconds to wait for clients to shut down gracefully during server destruction.
			// A negative value means infinite timeout. Zero means instant shutdown.
			int shutdown_timeout = 0;

			constexpr Options() noexcept {}
		};

		// Creates a local server.
		static std::unique_ptr<Server> create_local(Callbacks&, const std::string& name, const Options& = {});

		// Creates a TCP server.
		static std::unique_ptr<Server> create_tcp(Callbacks&, uint16_t port, const Options& = {});

		virtual ~Server() = default;
	};
}
