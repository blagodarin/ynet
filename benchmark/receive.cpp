#include "receive.h"

ReceiveClient::ReceiveClient(const std::string& host, uint16_t port, int64_t seconds, size_t bytes)
	: BenchmarkClient(host, port, seconds)
	, _bytes_per_mark(bytes)
{
}

void ReceiveClient::on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
}

void ReceiveClient::on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection, const void*, size_t size)
{
	if (stop_benchmark())
	{
		connection->close();
		return;
	}
	_bytes += size;
}

void ReceiveClient::on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&)
{
}

void ReceiveClient::on_failed_to_connect(const ynet::Client&)
{
	discard_benchmark();
}

ReceiveServer::ReceiveServer(uint16_t port, size_t bytes)
	: BenchmarkServer(port)
	, _buffer(bytes)
{
}

void ReceiveServer::on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>& connection)
{
	while (!connection->exhausted())
		connection->send(_buffer.data(), _buffer.size());
}

void ReceiveServer::on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void ReceiveServer::on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}
