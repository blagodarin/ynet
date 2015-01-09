#include "tcp_client.h"

#include <cassert>

namespace ynet
{
	class TcpClientSocket: public Socket
	{
	public:

		TcpClientSocket(TcpClient& client, const std::string& address, int port)
			: _client(client)
			, _address(address)
			, _port(port)
		{
		}

		std::string address() const override
		{
			return _address;
		}

		void close() override
		{
			// TODO: Implement.
		}

		int port() const override
		{
			return _port;
		}

		bool send(const void* data, size_t size) override
		{
			return _client.send(data, size);
		}

	private:

		TcpClient& _client;
		const std::string& _address;
		const int _port;
	};

	TcpClient::TcpClient(ClientCallbacks& callbacks, const std::string& host, int port)
		: _callbacks(callbacks)
		, _host(host)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _port_string(_port >= 0 ? std::to_string(_port) : std::string())
		, _reconnect_timeout(1000)
		, _socket(TcpBackend::InvalidSocket)
		, _closing(false)
	{
		if (_port < 0)
			return; // TODO: Throw.
		_thread = std::thread(std::bind(&TcpClient::run, this));
	}

	TcpClient::~TcpClient()
	{
		// TODO: Fix closing from the client thread.
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_closing = true;
			if (_socket != TcpBackend::InvalidSocket)
				TcpBackend::shutdown(_socket);
		}
		_closing_event.notify_one();
		_thread.join();
		assert(_socket == TcpBackend::InvalidSocket);
	}

	bool TcpClient::send(const void* data, size_t size)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_socket == TcpBackend::InvalidSocket || _closing)
			return false;
		auto block = static_cast<const uint8_t*>(data);
		while (size > 0)
		{
			const size_t block_size = std::min(_buffer.size(), size);
			if (!TcpBackend::send(_socket, block, block_size))
				return false;
			block += block_size;
			size -= block_size;
		}
		return true;
	}

	void TcpClient::run()
	{
		_callbacks.on_started(_host, _port);
		Link link;
		for (bool initial = true; ; )
		{
			const auto socket = TcpBackend::connect(_host, _port_string, link);
			if (socket != TcpBackend::InvalidSocket)
			{
				initial = false;
				{
					std::lock_guard<std::mutex> lock(_mutex);
					if (_closing)
					{
						TcpBackend::close(socket);
						break;
					}
					_socket = socket;
				}
				TcpClientSocket client_socket(*this, link.remote_address, link.remote_port);
				_callbacks.on_connected(link, client_socket);
				for (;;)
				{
					const size_t size = TcpBackend::recv(_socket, _buffer.data(), _buffer.size(), nullptr);
					if (size == 0)
						break;
					_callbacks.on_received(link, client_socket, _buffer.data(), size);
				}
				{
					std::lock_guard<std::mutex> lock(_mutex);
					TcpBackend::close(_socket);
					_socket = TcpBackend::InvalidSocket;
				}
				_callbacks.on_disconnected(link, client_socket);
			}
			else if (initial)
			{
				initial = false;
				_callbacks.on_failed_to_connect(_host, _port);
			}

			std::unique_lock<std::mutex> lock(_mutex);
			if (_reconnect_timeout > 0)
			{
				if (_closing_event.wait_for(lock, std::chrono::milliseconds(_reconnect_timeout), [this]() { return _closing; }))
					break;
			}
			else if (_closing)
				break;
		}
		_callbacks.on_stopped(_host, _port);
	}
}
