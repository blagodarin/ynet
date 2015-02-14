#include "send.h"

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

SendClient::SendClient(uint16_t port, int64_t seconds, size_t bytes, bool optimized_loopback)
	: BenchmarkClient(port, seconds, ::client_options(optimized_loopback))
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

SendServer::SendServer(uint16_t port, bool optimized_loopback)
	: BenchmarkServer(port, ::server_options(optimized_loopback))
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
