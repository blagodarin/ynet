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
	class ClientCallback
	{
	public:

		virtual ~ClientCallback() = default;

		//!
		//! \note The function is called from the client thread.
		virtual void on_connected(const Link& link, Socket& socket) = 0;

		//!
		//! \note The function is called from the client thread.
		virtual void on_disconnected(const Link& link) = 0;

		//!
		//! \note The function is called from the client thread.
		virtual void on_received(const Link& link, const void* data, size_t size, Socket& socket) = 0;

		//!
		//! \note The function is called from the client thread.
		virtual void on_refused(const std::string& host, int port) = 0;
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
	class ServerCallback
	{
	public:

		virtual ~ServerCallback() = default;

		//!
		//! \note The function is called from the server thread.
		virtual void on_connected(const Link& link, Socket& socket) = 0;

		//!
		//! \note The function is called from the server thread.
		virtual void on_disconnected(const Link& link) = 0;

		//!
		//! \note The function is called from the server thread.
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
