//! \file
//! \brief

#pragma once

#include <cstdint>
#include <memory>
#include <string>

//!
namespace ynet
{
	//! Connection with a remote party.
	class Connection
	{
	public:

		virtual ~Connection() = default;

		//!
		virtual void abort() = 0;

		//! Get the address of the remote party.
		virtual std::string address() const = 0;

		//! Close the connection.
		virtual void close() = 0;

		//! Send a message to the remote party synchronously.
		//! \return \c true if the message was sent.
		virtual bool send(const void* data, size_t size) = 0;
	};

	//! Network client.
	class Client
	{
	public:

		//!
		//! \note All callbacks are called from the client thread.
		class Callbacks
		{
		public:

			virtual ~Callbacks() = default;

			//!
			virtual void on_started(const Client& client);

			//!
			virtual void on_failed_to_connect(const Client& client);

			//!
			virtual void on_connected(const Client& client, const std::shared_ptr<Connection>& connection) = 0;

			//!
			virtual void on_received(const Client& client, const std::shared_ptr<Connection>& connection, const void* data, size_t size) = 0;

			//!
			virtual void on_disconnected(const Client& client, const std::shared_ptr<Connection>& connection) = 0;

			//!
			virtual void on_stopped(const Client& client);
		};

		//!
		struct Options
		{
			unsigned reconnect_timeout; //!<
			bool optimized_loopback; //!<

			Options();
		};

		//!
		static std::unique_ptr<Client> create(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options = {});

		virtual ~Client() = default;

		//! Get the host to connect to.
		//! \return Host as specified in create.
		virtual std::string host() const = 0;

		//! Get the port to connect to.
		//! \return Port.
		virtual uint16_t port() const = 0;
	};

	//! Network server.
	class Server
	{
	public:

		//!
		//! \note All callbacks are called from the server thread.
		class Callbacks
		{
		public:

			virtual ~Callbacks() = default;

			//!
			virtual void on_failed_to_start(const Server& server);

			//!
			virtual void on_started(const Server& server);

			//!
			virtual void on_connected(const Server& server, const std::shared_ptr<Connection>& connection) = 0;

			//!
			virtual void on_received(const Server& server, const std::shared_ptr<Connection>& connection, const void* data, size_t size) = 0;

			//!
			virtual void on_disconnected(const Server& server, const std::shared_ptr<Connection>& connection) = 0;

			//!
			virtual void on_stopped(const Server& server);
		};

		//!
		struct Options
		{
			bool optimized_loopback; //!<

			Options();
		};

		//!
		static std::unique_ptr<Server> create(Callbacks& callbacks, uint16_t port, const Options& options = Options());

		virtual ~Server() = default;

		//!
		virtual std::string address() const = 0;

		//!
		virtual std::string name() const = 0;

		//!
		virtual uint16_t port() const = 0;
	};
}
