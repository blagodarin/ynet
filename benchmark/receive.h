#pragma once

#include <vector>

#include "benchmark.h"

class ReceiveClient : public BenchmarkClient
{
public:
	ReceiveClient(const ClientFactory&, int64_t seconds, size_t bytes);

	uint64_t bytes() const { return _bytes; }
	uint64_t marks() const { return _bytes / _bytes_per_mark; }

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&, int&) override;
	void on_failed_to_connect(int&) override;

private:
	const size_t _bytes_per_mark;
	uint64_t _bytes = 0;
};

class ReceiveServer : public BenchmarkServer
{
public:
	ReceiveServer(const ServerFactory&, size_t bytes);
	~ReceiveServer() override { stop(); }

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&) override;

private:
	std::vector<uint8_t> _buffer;
};
