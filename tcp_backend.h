#pragma once

#include <string>
#include <vector>

#include "ynet.h"

namespace ynet
{
	namespace TcpBackend
	{
		typedef int Socket;

		const Socket InvalidSocket = -1;

		enum
		{
			SocketReceived = 1 << 0,
			SocketDisconnected = 1 << 1,
		};

		void close(Socket socket);
		Socket connect(const std::string& host, const std::string& port, Link& link);
		Socket listen(int port, std::string& address);
		size_t recv(Socket socket, void* data, size_t size, bool* disconnected);
		bool send(Socket socket, const void* data, size_t size);
		void shutdown(Socket socket);
		void shutdown_server(Socket socket);

		class Poller
		{
		public:

			class Callbacks
			{
			public:

				virtual void on_connected(Socket socket, std::string&& address, int port) = 0;
				virtual void on_received(Socket socket, bool& disconnected) = 0;
				virtual void on_disconnected(Socket socket) = 0;
			};

			Poller(Socket socket, Callbacks& callbacks): _listening_socket(socket), _callbacks(callbacks) {}

			explicit operator bool() const { return _listening_socket != InvalidSocket || !_peers.empty(); }

			void poll();

		private:

			void accept();

		private:

			Socket _listening_socket;
			Callbacks& _callbacks;
			std::vector<Socket> _peers;
			std::vector<std::tuple<Socket, bool, bool>> _triggered_peers;
		};
	}
}
