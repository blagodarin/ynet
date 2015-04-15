#pragma once

#include "benchmark.h"

class ConnectDisconnectClient: public BenchmarkClient
{
public:

	ConnectDisconnectClient(uint16_t port, int64_t seconds, bool optimized_loopback);

	const unsigned marks() const { return _marks; }

private:

	void on_failed_to_connect(const ynet::Client&) override;
	void on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;

private:

	unsigned _marks = 0;
};

class ConnectDisconnectServer: public BenchmarkServer
{
public:

	ConnectDisconnectServer(uint16_t port, bool optimized_loopback);
	~ConnectDisconnectServer() override { stop(); }

private:

	void on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
};
