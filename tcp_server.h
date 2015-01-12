#pragma once

#include <map>
#include <thread>

#include "tcp_backend.h"

namespace ynet
{
	class TcpServer: public Server, private TcpBackend::Poller::Callbacks
	{
	public:

		TcpServer(ServerCallbacks& callbacks, int port);
		~TcpServer() override;

		// Server
		std::string address() const override;
		std::string name() const override;
		int port() const override;

	private:

		void run();

		// TcpBackend::Poller::Callbacks
		void on_connected(TcpBackend::Socket socket, std::string&& address, int port, std::string&& name) override;
		void on_received(TcpBackend::Socket socket, bool& disconnected) override;
		void on_disconnected(TcpBackend::Socket socket) override;

	private:

		class Peer: public Client
		{
		public:

			Peer(TcpBackend::Socket socket, std::string&& address, int port, std::string&& name);
			Peer(const Peer&) = delete;
			Peer(Peer&&) = default;
			Peer& operator=(const Peer&) = delete;
			Peer& operator=(Peer&&) = default;

			// Client
			std::string address() const override;
			void disconnect() override;
			std::string host() const override;
			std::string name() const override;
			int port() const override;
			bool send(const void* data, size_t size) override;

			explicit operator bool() const;

		private:

			TcpBackend::Socket _socket;
			const std::string _address;
			const int _port;
			const std::string _name;
		};

		ServerCallbacks& _callbacks;
		const int _port;
		std::string _address;
		std::string _name;
		TcpBackend::Socket _socket;
		std::map<TcpBackend::Socket, Peer> _peers;
		std::thread _thread;
		TcpBackend::Poller _poller;
		std::array<unsigned char, ReceiveBlockSize> _buffer;
	};
}
