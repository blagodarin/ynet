#pragma once

#include <vector>

#include "benchmark.h"

class ExchangeClient: public BenchmarkClient
{
public:

	ExchangeClient(const std::string& host, uint16_t port, int64_t seconds, size_t bytes);

	const unsigned marks() const { return _marks; }

private:

	void on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_failed_to_connect(const ynet::Client&) override;

private:

	std::vector<uint8_t> _buffer;
	size_t _offset = 0;
	unsigned _marks = 0;
};

class ExchangeServer: public BenchmarkServer
{
public:

	ExchangeServer(uint16_t port, size_t bytes);

private:

	void on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;

private:

	std::vector<uint8_t> _buffer;
	size_t _offset = 0;
};