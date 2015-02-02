#include "send_receive.h"

#include <cstring>

SendReceiveClient::SendReceiveClient(const std::string& host, uint16_t port, unsigned cycles, size_t bytes_per_cycle)
	: BenchmarkClient(host, port)
	, _cycles_left(cycles)
	, _buffer(bytes_per_cycle)
{
}

void SendReceiveClient::on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection)
{
	start_benchmark();
	connection->send(_buffer.data(), _buffer.size());
}

void SendReceiveClient::on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
{
	if (size > _buffer.size() - _offset)
		throw std::logic_error("Unexpected received data size");
	::memcpy(&_buffer[_offset], data, size);
	_offset += size;
	if (_offset == _buffer.size())
	{
		if (_cycles_left > 1)
		{
			--_cycles_left;
			_offset = 0;
			connection->send(_buffer.data(), _buffer.size());
		}
		else
		{
			_cycles_left = 0;
			stop_benchmark();
		}
	}
}

void SendReceiveClient::on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&)
{
	if (_cycles_left > 0)
		discard_benchmark();
}

void SendReceiveClient::on_failed_to_connect(const ynet::Client&)
{
	discard_benchmark();
}

SendReceiveServer::SendReceiveServer(uint16_t port, size_t bytes_per_cycle)
	: BenchmarkServer(port)
	, _buffer(bytes_per_cycle)
{
}

void SendReceiveServer::on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}

void SendReceiveServer::on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
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

void SendReceiveServer::on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&)
{
}
