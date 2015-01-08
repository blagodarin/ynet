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
	class ClientCallback
	{
	public:

		virtual ~ClientCallback() = default;

		//!
		virtual void on_started(const std::string& host, int port);

		//!
		virtual void on_connected(const Link& link, Socket& socket) = 0;

		//!
		virtual void on_disconnected(const Link& link) = 0;

		//!
		virtual void on_received(const Link& link, const void* data, size_t size, Socket& socket) = 0;

		//!
		virtual void on_refused(const std::string& host, int port);
	};

	//!
	class Client: public Socket
	{
	public:

		//!
		static std::unique_ptr<Client> create(ClientCallback& callback, const std::string& host, int port);

		~Client() override = default;

		//! Start the client activity.
		virtual bool start() = 0;
	};

	//!
	//! \note All callbacks are called from the server thread.
	class ServerCallback
	{
	public:

		virtual ~ServerCallback() = default;

		//!
		virtual void on_started(const Link& link);

		//!
		virtual void on_connected(const Link& link, Socket& socket) = 0;

		//!
		virtual void on_disconnected(const Link& link) = 0;

		//!
		virtual void on_received(const Link& link, const void* data, size_t size, Socket& socket) = 0;
	};

	//!
	class Server
	{
	public:

		//!
		static std::unique_ptr<Server> create(ServerCallback& callback, int port);

		virtual ~Server() = default;

		//! Start the server activity.
		virtual bool start() = 0;
	};
}
