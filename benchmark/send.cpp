#include "send.h"

SendClient::SendClient(const ClientFactory& factory, int64_t seconds, size_t bytes)
	: BenchmarkClient(factory, seconds)
	, _buffer(bytes)
{
	set_disconnect_timeout(-1);
}

void SendClient::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
	do
	{
		connection->send(_buffer.data(), _buffer.size());
		++_marks;
	} while (!stop_benchmark());
}

void SendClient::on_received(const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
{
}

void SendClient::on_disconnected(const std::shared_ptr<ynet::Connection>&, int&)
{
}

void SendClient::on_failed_to_connect(int&)
{
	discard_benchmark();
}

SendServer::SendServer(const ServerFactory& factory)
	: BenchmarkServer(factory)
{
}

void SendServer::on_connected(const std::shared_ptr<ynet::Connection>&)
{
}

void SendServer::on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void SendServer::on_disconnected(const std::shared_ptr<ynet::Connection>&)
{
}
