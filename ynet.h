#pragma once

#include <memory>
#include <string>

namespace ynet
{
	//!
	class Socket
	{
	public:

		virtual ~Socket() = default;

		//! Close the socket.
		virtual void close() = 0;

		//! Send a message to the remote party synchronously.
		//! \return \c true if the message was sent.
		virtual bool send(const void* data, size_t size) = 0;
	};

	//!
	struct Link
	{
		//!
		std::string local_address;

		//!
		int local_port;

		//!
		std::string remote_address;

		//!
		int remote_port;

		Link(): local_port(-1), remote_port(-1) {}
	};

	//!
	//! \note All callbacks are called from the client thread.
	class ClientCallbacks
	{
	public:

		virtual ~ClientCallbacks() = default;

		//!
		virtual void on_started(const std::string& host, int port);

		//!
		virtual void on_connected(const Link& link, Socket& socket) = 0;

		//!
		virtual void on_disconnected(const Link& link) = 0;

		//!
		virtual void on_received(const Link& link, const void* data, size_t size, Socket& socket) = 0;

		//!
		virtual void on_failed_to_connect(const std::string& host, int port);

		//!
		virtual void on_stopped(const std::string& host, int port);
	};

	//!
	class Client: public Socket
	{
	public:

		//!
		static std::unique_ptr<Client> create(ClientCallbacks& callbacks, const std::string& host, int port);

		~Client() override = default;
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
		virtual void on_connected(const Server& server, const std::string& address, int port, Socket& client) = 0;

		//!
		virtual void on_disconnected(const Server& server, const std::string& address, int port) = 0;

		//!
		virtual void on_received(const Server& server, const std::string& address, int port, const void* data, size_t size, Socket& client) = 0;

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
		virtual int port() const = 0;
	};
}
