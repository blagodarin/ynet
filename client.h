#pragma once

#include <memory>
#include <string>

namespace ynet
{
	class ClientCallback;

	//!
	class Client
	{
	public:

		//!
		static std::unique_ptr<Client> create(ClientCallback& callback, const std::string& host, int port);

		virtual ~Client() = default;

		//! Open the socket if it hasn't been open yet.
		virtual bool open() = 0;

		//! Close the socket.
		virtual void close() = 0;

		//! Send a message to the server synchronously.
		//! \return \c true if the message was sent.
		virtual bool send(const void* data, size_t size) = 0;

		//!
		virtual std::string remote_host() const = 0;

		//!
		virtual int remote_port() const = 0;
	};

	struct ClientConnection
	{
		std::string local_address;
		int         local_port;
		std::string remote_address;
		int         remote_port;

		ClientConnection(): local_port(-1), remote_port(-1) {}
	};

	//!
	class ClientCallback
	{
	public:

		virtual ~ClientCallback() = default;

		//!
		//! \note The function is called from the \a client thread.
		virtual void on_connect(Client& client, const ClientConnection& connection) = 0;

		//!
		//! \note The function is called from the \a client thread.
		virtual void on_disconnect(const Client& client, const ClientConnection& connection) = 0;

		//!
		//! \note The function is called from the \a client thread.
		virtual void on_receive(Client& client, const ClientConnection& connection, const void* data, size_t size) = 0;

		//!
		//! \note The function is called from the \a client thread.
		virtual void on_refuse(const Client& client) = 0;
	};
}
