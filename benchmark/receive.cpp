#include "receive.h"

ReceiveClient::ReceiveClient(uint16_t port, int64_t seconds, size_t bytes, ynet::Protocol protocol)
	: BenchmarkClient(port, seconds, protocol)
	, _bytes_per_mark(bytes)
{
	// The server sends us data as long as it can, so infinite wait for graceful disconnect is not an option.
}

void ReceiveClient::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
}

void ReceiveClient::on_received(const std::shared_ptr<ynet::Connection>& connection, const void*, size_t size)
{
	if (stop_benchmark())
	{
		connection->close();
		return;
	}
	_bytes += size;
}

void ReceiveClient::on_disconnected(const std::shared_ptr<ynet::Connection>&, int&)
{
}

void ReceiveClient::on_failed_to_connect(int&)
{
	discard_benchmark();
}

ReceiveServer::ReceiveServer(uint16_t port, size_t bytes, ynet::Protocol protocol)
	: BenchmarkServer(port, protocol)
	, _buffer(bytes)
{
}

void ReceiveServer::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	while (connection->send(_buffer.data(), _buffer.size()));
}

void ReceiveServer::on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
}

void ReceiveServer::on_disconnected(const std::shared_ptr<ynet::Connection>&)
{
}
