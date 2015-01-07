#include "client.h"

#include <condition_variable>
#include <mutex>
#include <thread>

#include "tcp_client_posix.h"

namespace ynet
{
	class TcpClient: public Client
	{
	public:

		TcpClient(ClientCallback& callback, const std::string& host, int port);
		~TcpClient() override;

		bool open() override;
		void close() override;
		bool send(const void* data, size_t size) override;
		std::string remote_host() const override;
		int remote_port() const override;

	private:

		void run();

	private:
	
		ClientCallback& _callback;
		const std::string _host;
		const int _port;
		const std::string _port_string;
		bool _open;
		std::thread _thread;
		std::mutex _mutex;
		TcpClientBackend::Socket _socket;
		bool _closing;
		std::condition_variable _closing_event;
		std::array<unsigned char, 48 * 1024> _buffer;
	};
}
