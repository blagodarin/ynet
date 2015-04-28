#include "exchange.h"

ExchangeClient::ExchangeClient(uint16_t port, int64_t seconds, size_t bytes, ynet::Protocol protocol)
	: BenchmarkClient(port, seconds, protocol)
	, _buffer(bytes)
{
	set_disconnect_timeout(-1);
}

void ExchangeClient::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
	connection->send(_buffer.data(), _buffer.size());
}

void ExchangeClient::on_received(const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
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

void ExchangeClient::on_disconnected(const std::shared_ptr<ynet::Connection>&, int&)
{
}

void ExchangeClient::on_failed_to_connect(int&)
{
	discard_benchmark();
}

ExchangeServer::ExchangeServer(uint16_t port, size_t bytes, ynet::Protocol protocol)
	: BenchmarkServer(port, protocol)
	, _buffer(bytes)
{
}

void ExchangeServer::on_connected(const std::shared_ptr<ynet::Connection>&)
{
}

void ExchangeServer::on_received(const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
{
	if (size > _buffer.size() - _offset)
		throw std::logic_error("Unexpected received data size");
	_offset += size;
	if (_offset < _buffer.size())
		return;
	_offset = 0;
	connection->send(_buffer.data(), _buffer.size());
}

void ExchangeServer::on_disconnected(const std::shared_ptr<ynet::Connection>&)
{
}
