#pragma once

#include "benchmark.h"

class ConnectDisconnectClient : public BenchmarkClient
{
public:
	ConnectDisconnectClient(const ClientFactory&, int64_t seconds);

	const unsigned marks() const { return _marks; }

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&, int&) override;
	void on_failed_to_connect(int&) override;

private:
	unsigned _marks = 0;
};

class ConnectDisconnectServer : public BenchmarkServer
{
public:
	ConnectDisconnectServer(const ServerFactory&);
	~ConnectDisconnectServer() override { stop(); }

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&) override;
};
