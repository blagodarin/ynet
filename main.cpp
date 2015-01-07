#include <cstring>
#include <iomanip>
#include <iostream>

#include <thread>

#include "client.h"

namespace
{
	void dump(const char* data, size_t size)
	{
		static const size_t line_size = 32;
		char buffer[line_size + 1];
		buffer[line_size] = '\0';
		while (size > 0)
		{
			const auto part_size = std::min(size, line_size);
			::memcpy(buffer, data, part_size);
			data += part_size;
			size -= part_size;

			std::cout << std::hex << "\t" << std::setfill('0');
			for (size_t i = 0; i < part_size; ++i)
			{
				const auto n = static_cast<int>(buffer[i]);
				std::cout << std::setw(2) << n << ' ';
				if (n < 32 || n >127)
					buffer[i] = '.';
			}
			std::cout << std::setfill(' ') << std::setw(2 + (line_size - part_size) * 3) << ' ';
			for (size_t i = 0; i < part_size; ++i)
				std::cout << buffer[i];
			std::cout << std::dec << std::endl;
		}
	}
}

class Client: public ynet::ClientCallback
{
public:

	Client(const std::string& host, int port)
		: _client(ynet::Client::create(*this, host, port))
	{
		std::cout << "Started" << std::endl;
		_client->open();
	}

	~Client() override
	{
		_client->close();
		std::cout << "Stopped" << std::endl;
	}

private:

	void on_connect(ynet::Client& client, const ynet::ClientConnection& connection) override
	{
		std::cout << "Connected to " << connection.remote_address << ":" << connection.remote_port
			<< " (as " << connection.local_address << ":" << connection.local_port << ")" << std::endl;
		//std::this_thread::sleep_for(std::chrono::seconds(5));
		client.send("GET / HTTP/1.1\r\n\r\n", 18);
	}

	void on_disconnect(const ynet::Client&, const ynet::ClientConnection& connection) override
	{
		std::cout << "Disconnected from " << connection.remote_address << ":" << connection.remote_port
			<< " (as " << connection.local_address << ":" << connection.local_port << ")" << std::endl;
	}

	void on_receive(ynet::Client&, const ynet::ClientConnection& connection, const void* data, size_t size) override
	{
		std::cout << "Received " << size << " bytes from " << connection.remote_address << ":" << connection.remote_port
			<< " (as " << connection.local_address << ":" << connection.local_port << ")" << std::endl;
		::dump(static_cast<const char*>(data), size);
	}

	void on_refuse(const ynet::Client& client) override
	{
		std::cout << "Connection to " << client.remote_host() << ":" << client.remote_port() << " failed" << std::endl;
	}

private:

	std::unique_ptr<ynet::Client> _client;
};

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " HOST PORT" << std::endl;
		return 1;
	}
	Client client(argv[1], ::atoi(argv[2]));
	std::cin.get();
	return 0;
}
