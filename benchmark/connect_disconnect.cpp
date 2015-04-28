#include "connect_disconnect.h"

ConnectDisconnectClient::ConnectDisconnectClient(uint16_t port, int64_t seconds, ynet::Protocol protocol)
	: BenchmarkClient(port, seconds, protocol)
{
	set_disconnect_timeout(-1);
}

void ConnectDisconnectClient::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	connection->close();
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

ConnectDisconnectServer::ConnectDisconnectServer(uint16_t port, ynet::Protocol protocol)
	: BenchmarkServer(port, protocol)
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
