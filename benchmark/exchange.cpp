#include "exchange.h"

#include <cstring>

ExchangeClient::ExchangeClient(const std::string& host, uint16_t port, int64_t seconds, size_t bytes)
	: BenchmarkClient(host, port, seconds)
	, _buffer(bytes)
{
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
	::memcpy(&_buffer[_offset], data, size);
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

void ExchangeClient::on_failed_to_connect(const ynet::Client&)
{
	discard_benchmark();
}

ExchangeServer::ExchangeServer(uint16_t port, size_t bytes)
	: BenchmarkServer(port)
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
	::memcpy(&_buffer[_offset], data, size);
	_offset += size;
	if (_offset == _buffer.size())
	{
		_offset = 0;
		connection->send(_buffer.data(), _buffer.size());
	}
}

void ExchangeServer::on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}
