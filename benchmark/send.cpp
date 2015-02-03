#include "send.h"

SendClient::SendClient(const std::string& host, uint16_t port, int64_t seconds, size_t bytes)
	: BenchmarkClient(host, port, seconds)
	, _buffer(bytes)
{
}

void SendClient::on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
	do
	{
		connection->send(_buffer.data(), _buffer.size());
		++_marks;
	} while (!stop_benchmark());
}

void SendClient::on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
{
}

void SendClient::on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&)
{
}

void SendClient::on_failed_to_connect(const ynet::Client&)
{
	discard_benchmark();
}

SendServer::SendServer(uint16_t port)
	: BenchmarkServer(port)
{
}

void SendServer::on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}

void SendServer::on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void SendServer::on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}
