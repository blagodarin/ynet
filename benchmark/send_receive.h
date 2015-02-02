#pragma once

#include <vector>

#include "benchmark.h"

class SendReceiveClient: public BenchmarkClient
{
public:

	SendReceiveClient(const std::string& host, uint16_t port, unsigned cycles, size_t bytes_per_cycle);

private:

	void on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_failed_to_connect(const ynet::Client&) override;

private:

	unsigned _cycles_left;
	std::vector<uint8_t> _buffer;
	size_t _offset = 0;
};

class SendReceiveServer: public BenchmarkServer
{
public:

	SendReceiveServer(uint16_t port, size_t bytes_per_cycle);

private:

	void on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;

private:

	std::vector<uint8_t> _buffer;
	size_t _offset = 0;
};
