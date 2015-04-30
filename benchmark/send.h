#pragma once

#include <vector>

#include "benchmark.h"

class SendClient : public BenchmarkClient
{
public:

	SendClient(const ClientFactory&, int64_t seconds, size_t bytes);

	uint64_t bytes() const { return _marks * _buffer.size(); }
	uint64_t marks() const { return _marks; }

private:

	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&, int&) override;
	void on_failed_to_connect(int&) override;

private:

	std::vector<uint8_t> _buffer;
	size_t _offset = 0;
	uint64_t _marks = 0;
};

class SendServer : public BenchmarkServer
{
public:

	SendServer(const ServerFactory&);
	~SendServer() override { stop(); }

private:

	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&) override;
};
