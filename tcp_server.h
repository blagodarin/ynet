#include "ynet.h"

namespace ynet
{
	class TcpServer: public Server
	{
	public:

		TcpServer(ServerCallback& callback, int port);
		~TcpServer() override;

		// Server
		bool start() override;

	private:

		ServerCallback& _callback;
		const int _port;
		const std::string _port_string;
	};
}
