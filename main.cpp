#include <cstring>
#include <iomanip>
#include <iostream>

#include "ynet.h"

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
				if (n < 32 || n > 127)
					buffer[i] = '.';
			}
			std::cout << std::setfill(' ') << std::setw(2 + (line_size - part_size) * 3) << ' ';
			for (size_t i = 0; i < part_size; ++i)
				std::cout << buffer[i];
			std::cout << std::dec << std::endl;
		}
	}
}

class Client: public ynet::Client::Callbacks
{
public:

	Client(const std::string& host, int port)
		: _client(ynet::Client::create(*this, host, port))
	{
	}

private:

	void on_started(const ynet::Client& client) override
	{
		std::cout << "Started connecting to " << client.host() << ":" << client.port() << std::endl;
	}

	void on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection) override
	{
		std::cout << "Connected to " << connection->name() << std::endl;
		const std::string request = "GET / HTTP/1.1\r\n\r\n";
		connection->send(request.data(), request.size());
	}

	void on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size) override
	{
		std::cout << "Received " << size << " bytes from " << connection->name() << std::endl;
		::dump(static_cast<const char*>(data), size);
	}

	void on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection) override
	{
		std::cout << "Disconnected from " << connection->name() << std::endl;
	}

	void on_failed_to_connect(const ynet::Client& client) override
	{
		std::cout << "Failed to connect to " << client.host() << ":" << client.port() << std::endl;
	}

	void on_stopped(const ynet::Client& client) override
	{
		std::cout << "Stopped connecting to " << client.host() << ":" << client.port() << std::endl;
	}

private:

	std::unique_ptr<ynet::Client> _client;
};

class Server: public ynet::Server::Callbacks
{
public:

	Server(int port)
		: _server(ynet::Server::create(*this, port))
	{
	}

private:

	void on_started(const ynet::Server& server) override
	{
		std::cout << "Server " << server.name() << " started" << std::endl;
	}

	void on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>& connection) override
	{
		std::cout << "Client " << connection->name() << " connected" << std::endl;
	}

	void on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size) override
	{
		std::cout << "Client " << connection->name() << " sent " << size << " bytes" << std::endl;
		::dump(static_cast<const char*>(data), size);
		const std::string reply = "You are " + connection->name() + "\r\n\r\n";
		connection->send(reply.data(), reply.size());
	}

	void on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>& connection) override
	{
		std::cout << "Client " << connection->name() << " disconnected" << std::endl;
	}

	void on_stopped(const ynet::Server& server) override
	{
		std::cout << "Server " << server.name() << " stopped" << std::endl;
	}

private:

	std::unique_ptr<ynet::Server> _server;
};

int main(int argc, char** argv)
{
	if (argc == 3)
	{
		Client client(argv[1], ::atoi(argv[2]));
		std::cin.get();
		return 0;
	}
	else if (argc == 2)
	{
		Server server(::atoi(argv[1]));
		std::cin.get();
		return 0;
	}
	else
	{
		std::cerr << "Usage:\n\tynet HOST PORT\n\tynet PORT" << std::endl;
		return 1;
	}
}
