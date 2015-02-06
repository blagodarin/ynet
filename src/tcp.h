#pragma once

#include "client.h"
#include "server.h"

namespace ynet
{
	class TcpServer: public ServerImpl
	{
	public:

		TcpServer(Callbacks& callbacks, uint16_t port, const Options& options);
		~TcpServer() override;

	protected:

		bool listen(const ::sockaddr_storage& _sockaddr) override;
		void poll() override;
		void shutdown() override;

	private:

		struct Private;
		std::unique_ptr<Private> _private;
	};

	std::unique_ptr<ConnectionImpl> create_tcp_connection(const ::sockaddr_storage& sockaddr);
}
