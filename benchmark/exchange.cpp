#include "exchange.h"

#include <cstring>

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

ExchangeClient::ExchangeClient(uint16_t port, int64_t seconds, size_t bytes, bool optimized_loopback)
	: BenchmarkClient(port, seconds, ::client_options(optimized_loopback))
	, _buffer(bytes)
{
}

void ExchangeClient::on_failed_to_connect(const ynet::Client&)
{
	discard_benchmark();
}

void ExchangeClient::on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
	connection->send(_buffer.data(), _buffer.size());
}

void ExchangeClient::on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
{
	if (size > _buffer.size() - _offset)
		throw std::logic_error("Unexpected received data size");
	_offset += size;
	if (_offset < _buffer.size())
		return;
	++_marks;
	if (stop_benchmark())
		return;
	_offset = 0;
	connection->send(_buffer.data(), _buffer.size());
}

void ExchangeClient::on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&)
{
}

ExchangeServer::ExchangeServer(uint16_t port, size_t bytes, bool optimized_loopback)
	: BenchmarkServer(port, ::server_options(optimized_loopback))
	, _buffer(bytes)
{
}

void ExchangeServer::on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}

void ExchangeServer::on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
{
	if (size > _buffer.size() - _offset)
		throw std::logic_error("Unexpected received data size");
	_offset += size;
	if (_offset < _buffer.size())
		return;
	_offset = 0;
	connection->send(_buffer.data(), _buffer.size());
}

void ExchangeServer::on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}
