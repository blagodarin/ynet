#include "connect_disconnect.h"

namespace
{
	const auto client_options = []
	{
		ynet::Client::Options options;
		options.shutdown_timeout = -1;
		return options;
	}();
}

ConnectDisconnectClient::ConnectDisconnectClient(const ClientFactory& factory, int64_t seconds)
	: BenchmarkClient(factory, client_options, seconds)
{
}

void ConnectDisconnectClient::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	connection->shutdown();
}

void ConnectDisconnectClient::on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void ConnectDisconnectClient::on_disconnected(const std::shared_ptr<ynet::Connection>&, int& reconnect_timeout)
{
	++_marks;
	stop_benchmark();
	reconnect_timeout = 0;
}

void ConnectDisconnectClient::on_failed_to_connect(int&)
{
	discard_benchmark();
}

ConnectDisconnectServer::ConnectDisconnectServer(const ServerFactory& factory)
	: BenchmarkServer(factory)
{
}

void ConnectDisconnectServer::on_connected(const std::shared_ptr<ynet::Connection>&)
{
}

void ConnectDisconnectServer::on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void ConnectDisconnectServer::on_disconnected(const std::shared_ptr<ynet::Connection>&)
{
}
