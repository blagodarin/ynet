#pragma once

#include "client.h"
#include "server.h"

namespace ynet
{
	class TcpClient: public ClientImpl
	{
	public:

		TcpClient(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options);
		~TcpClient() override;

	protected:

		std::unique_ptr<ConnectionImpl> connect(const ::sockaddr_storage& sockaddr) override;
		std::unique_ptr<ConnectionImpl> connect_local(uint16_t) override;
	};

	class TcpServer: public ServerImpl
	{
	public:

		TcpServer(Callbacks& callbacks, uint16_t port);
		~TcpServer() override;

	protected:

		bool listen(const ::sockaddr_storage& _sockaddr) override;
		void poll() override;
		void shutdown() override;

	private:

		struct Private;
		std::unique_ptr<Private> _private;
	};
}
