#pragma once

#include "benchmark.h"

class ConnectDisconnectClient: public BenchmarkClient
{
public:

	ConnectDisconnectClient(const std::string& host, uint16_t port, unsigned attempts);

private:

	void on_connected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Client&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Client&, const std::shared_ptr<ynet::Connection>&) override;
	void on_failed_to_connect(const ynet::Client&) override;

private:

	unsigned _attempts_left;
};

class ConnectDisconnectServer: public BenchmarkServer
{
public:

	ConnectDisconnectServer(uint16_t port);

private:

	void on_connected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const ynet::Server&, const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const ynet::Server&, const std::shared_ptr<ynet::Connection>&) override;
};
