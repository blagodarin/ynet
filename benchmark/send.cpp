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

SendClient::SendClient(uint16_t port, int64_t seconds, size_t bytes, unsigned flags)
	: BenchmarkClient(port, seconds, ::client_options(flags & BenchmarkLocal))
	, _flags(flags)
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

SendServer::SendServer(uint16_t port, unsigned flags)
	: BenchmarkServer(port, ::server_options(flags & BenchmarkLocal))
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
