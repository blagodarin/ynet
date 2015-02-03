#include "connect_disconnect.h"

namespace
{
	ynet::Client::Options client_options()
	{
		ynet::Client::Options options;
		options.reconnect_timeout = 0;
		return options;
	}
}

ConnectDisconnectClient::ConnectDisconnectClient(const std::string& host, uint16_t port, int64_t seconds)
	: BenchmarkClient(host, port, seconds, client_options())
{
}

void ConnectDisconnectClient::on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection)
{
	connection->close();
}

void ConnectDisconnectClient::on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void ConnectDisconnectClient::on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&)
{
	++_marks;
	stop_benchmark();
}

void ConnectDisconnectClient::on_failed_to_connect(const ynet::Client&)
{
	discard_benchmark();
}

ConnectDisconnectServer::ConnectDisconnectServer(uint16_t port)
	: BenchmarkServer(port)
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