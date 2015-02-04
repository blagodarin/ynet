#pragma once

#include "client.h"
#include "server.h"

namespace ynet
{
	class TcpClient: public ClientImpl
	{
	public:

		TcpClient(Callbacks& callbacks, const std::string& host, uint16_t port, const Options& options, Trigger& trigger);
		~TcpClient() override;

	protected:

		std::unique_ptr<ConnectionImpl> connect(const ::sockaddr_storage& sockaddr) override;
		std::unique_ptr<ConnectionImpl> connect_local() override;
		size_t receive_buffer_size() const override;
	};

	class TcpServer: public ServerImpl
	{
	public:

		TcpServer(Callbacks& callbacks, uint16_t port, Trigger& trigger);
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
