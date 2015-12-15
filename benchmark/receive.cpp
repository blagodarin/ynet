#include "receive.h"

namespace
{
	const auto client_options = []
	{
		ynet::Client::Options options;
		options.shutdown_timeout = 0; // The server sends us data as long as it can, so infinite wait for graceful disconnect is not an option.
		return options;
	}();
}

ReceiveClient::ReceiveClient(const ClientFactory& factory, int64_t seconds, size_t bytes)
	: BenchmarkClient(factory, client_options, seconds)
	, _bytes_per_mark(bytes)
{
}

void ReceiveClient::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
}

void ReceiveClient::on_received(const std::shared_ptr<ynet::Connection>& connection, const void*, size_t size)
{
	if (stop_benchmark())
	{
		connection->shutdown();
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

ReceiveServer::ReceiveServer(const ServerFactory& factory, size_t bytes)
	: BenchmarkServer(factory)
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
