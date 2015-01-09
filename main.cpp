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

class Client: public ynet::ClientCallbacks
{
public:

	Client(const std::string& host, int port)
		: _client(ynet::Client::create(*this, host, port))
	{
		_client->start();
	}

private:

	void on_started(const std::string& host, int port) override
	{
		std::cout << "Started connecting to " << host << ":" << port << std::endl;
	}

	void on_connected(const ynet::Link& link, ynet::Socket& socket) override
	{
		std::cout << "Connected to " << link.remote_address << ":" << link.remote_port
			<< " (as " << link.local_address << ":" << link.local_port << ")" << std::endl;
		const std::string request = "GET / HTTP/1.1\r\n\r\n";
		socket.send(request.data(), request.size());
	}

	void on_disconnected(const ynet::Link& link) override
	{
		std::cout << "Disconnected from " << link.remote_address << ":" << link.remote_port
			<< " (as " << link.local_address << ":" << link.local_port << ")" << std::endl;
	}

	void on_received(const ynet::Link& link, const void* data, size_t size, ynet::Socket&) override
	{
		std::cout << "Received " << size << " bytes from " << link.remote_address << ":" << link.remote_port
			<< " (as " << link.local_address << ":" << link.local_port << ")" << std::endl;
		::dump(static_cast<const char*>(data), size);
	}

	void on_failed_to_connect(const std::string& host, int port) override
	{
		std::cout << "Failed to connect to " << host << ":" << port << std::endl;
	}

	void on_stopped(const std::string& host, int port) override
	{
		std::cout << "Stopped connecting to " << host << ":" << port << std::endl;
	}

private:

	std::unique_ptr<ynet::Client> _client;
};

class Server: public ynet::ServerCallbacks
{
public:

	Server(int port)
		: _server(ynet::Server::create(*this, port))
	{
		_server->start();
	}

private:

	void on_started(const std::string& address, int port) override
	{
		std::cout << "Server " << address << ":" << port << " started" << std::endl;
	}

	void on_connected(const ynet::Link& link, ynet::Socket&) override
	{
		std::cout << "Client " << link.remote_address << ":" << link.remote_port << " connected" << std::endl;
	}

	void on_disconnected(const ynet::Link& link) override
	{
		std::cout << "Client " << link.remote_address << ":" << link.remote_port << " disconnected" << std::endl;
	}

	void on_received(const ynet::Link& link, const void* data, size_t size, ynet::Socket& socket) override
	{
		std::cout << "Client " << link.remote_address << ":" << link.remote_port << " sent " << size << " bytes" << std::endl;
		::dump(static_cast<const char*>(data), size);
		const std::string reply = "You are " + link.remote_address + ':' + std::to_string(link.remote_port) + "\r\n\r\n";
		socket.send(reply.data(), reply.size());
	}

	void on_stopped(const std::string& address, int port) override
	{
		std::cout << "Server " << address << ":" << port << " stopped" << std::endl;
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
