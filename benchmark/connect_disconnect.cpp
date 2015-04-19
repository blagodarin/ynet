#include "connect_disconnect.h"

namespace
{
	ynet::Client::Options client_options(bool optimized_loopback)
	{
		ynet::Client::Options options;
		options.optimized_loopback = optimized_loopback;
		return options;
	}

	ynet::Server::Options server_options(bool optimized_loopback)
	{
		ynet::Server::Options options;
		options.optimized_loopback = optimized_loopback;
		return options;
	}
}

ConnectDisconnectClient::ConnectDisconnectClient(uint16_t port, int64_t seconds, bool optimized_loopback)
	: BenchmarkClient(port, seconds, client_options(optimized_loopback))
{
}

void ConnectDisconnectClient::on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection)
{
	connection->close();
}

void ConnectDisconnectClient::on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void ConnectDisconnectClient::on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, int& reconnect_timeout)
{
	++_marks;
	stop_benchmark();
	reconnect_timeout = 0;
}

void ConnectDisconnectClient::on_failed_to_connect(const ynet::Client&, bool, int&)
{
	discard_benchmark();
}

ConnectDisconnectServer::ConnectDisconnectServer(uint16_t port, bool optimized_loopback)
	: BenchmarkServer(port, server_options(optimized_loopback))
{
}

void ConnectDisconnectServer::on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}

void ConnectDisconnectServer::on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void ConnectDisconnectServer::on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}
