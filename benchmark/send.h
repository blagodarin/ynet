#pragma once

#include <vector>

#include "benchmark.h"

class SendClient : public BenchmarkClient
{
public:

	SendClient(uint16_t port, int64_t seconds, size_t bytes, unsigned flags);

	uint64_t bytes() const { return _marks * _buffer.size(); }
	uint64_t marks() const { return _marks; }

private:

	void on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, int&) override;
	void on_failed_to_connect(const ynet::Client&, bool, int&) override;

private:

	const unsigned _flags;
	std::vector<uint8_t> _buffer;
	size_t _offset = 0;
	uint64_t _marks = 0;
};

class SendServer : public BenchmarkServer
{
public:

	SendServer(uint16_t port, unsigned flags);
	~SendServer() override { stop(); }

private:

	void on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
};
