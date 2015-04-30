#include "dump.h"

#include <ynet.h>

class Client : public ynet::Client::Callbacks
{
public:

	Client(const std::string& host, uint16_t port)
		: _host(host)
		, _port(port)
		, _client(ynet::Client::create_tcp(*this, _host, _port)) {}

private:

	void on_started() override
	{
		std::cout << "Client started" << std::endl;
	}

	void on_connected(const std::shared_ptr<ynet::Connection>& connection) override
	{
		_initial_connect = false;
		std::cout << "Connected to " << connection->address() << std::endl;
		const std::string request = "GET / HTTP/1.1\r\n\r\n";
		connection->send(request.data(), request.size());
	}

	void on_received(const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size) override
	{
		std::cout << "Received " << size << " bytes from " << connection->address() << std::endl;
		::dump(static_cast<const char*>(data), size);
	}

	void on_disconnected(const std::shared_ptr<ynet::Connection>& connection, int& reconnect_timeout) override
	{
		std::cout << "Disconnected from " << connection->address() << std::endl;
		reconnect_timeout = 1000;
	}

	void on_failed_to_connect(int& reconnect_timeout) override
	{
		if (_initial_connect)
		{
			std::cout << "Failed to connect to " << _host << " (port " << _port << ")" << std::endl;
			_initial_connect = false;
		}
		reconnect_timeout = 1000;
	}

private:

	const std::string _host;
	const uint16_t _port;
	bool _initial_connect = true;
	std::unique_ptr<ynet::Client> _client;
};

class Server : public ynet::Server::Callbacks
{
public:

	Server(uint16_t port): _server(ynet::Server::create_tcp(*this, port)) {}

private:

	void on_failed_to_start(int& restart_timeout) override
	{
		if (_initial_startup)
		{
			std::cout << "Server failed to start" << std::endl;
			_initial_startup = false;
		}
		restart_timeout = 1000;
	}

	void on_started() override
	{
		std::cout << "Server started" << std::endl;
	}

	void on_connected(const std::shared_ptr<ynet::Connection>& connection) override
	{
		std::cout << "Client " << connection->address() << " connected" << std::endl;
	}

	void on_received(const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size) override
	{
		std::cout << "Client " << connection->address() << " sent " << size << " bytes" << std::endl;
		::dump(static_cast<const char*>(data), size);
		const std::string reply = "You are " + connection->address() + "\r\n\r\n";
		connection->send(reply.data(), reply.size());
	}

	void on_disconnected(const std::shared_ptr<ynet::Connection>& connection) override
	{
		std::cout << "Client " << connection->address() << " disconnected" << std::endl;
	}

private:

	bool _initial_startup = true;
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
