#pragma once

#include <condition_variable>
#include <mutex>

#include <ynet.h>

using ClientFactory = std::function<std::unique_ptr<ynet::Client>(ynet::Client::Callbacks&)>;
using ServerFactory = std::function<std::unique_ptr<ynet::Server>(ynet::Server::Callbacks&)>;

struct BenchmarkLocal
{
	static std::unique_ptr<ynet::Client> create_client(ynet::Client::Callbacks& callbacks)
	{
		return ynet::Client::create_local(callbacks, "ynet-benchmark");
	}

	static std::unique_ptr<ynet::Server> create_server(ynet::Server::Callbacks& callbacks)
	{
		return ynet::Server::create_local(callbacks, "ynet-benchmark");
	}
};

struct BenchmarkTcp
{
	static std::unique_ptr<ynet::Client> create_client(ynet::Client::Callbacks& callbacks)
	{
		return ynet::Client::create_tcp(callbacks, "localhost", 5445);
	}

	static std::unique_ptr<ynet::Server> create_server(ynet::Server::Callbacks& callbacks)
	{
		return ynet::Server::create_tcp(callbacks, 5445);
	}
};

class BenchmarkClient : public ynet::Client::Callbacks
{
public:

	BenchmarkClient(const ClientFactory&, int64_t seconds);

	int64_t run();

protected:

	void discard_benchmark();
	void set_shutdown_timeout(int milliseconds) { _shutdown_timeout = milliseconds; }
	void start_benchmark();
	bool stop_benchmark();

private:

	void on_started() final;

private:

	const ClientFactory _factory;
	std::mutex _mutex;
	bool _stop_flag = false;
	std::condition_variable _stop_condition;
	int64_t _benchmark_time;
	int64_t _start_time = 0;
	int64_t _elapsed_time = 0;
	bool _stopped = false;
	bool _discarded = false;
	int _shutdown_timeout = 0;
};

class BenchmarkServer : public ynet::Server::Callbacks
{
public:

	BenchmarkServer(const ServerFactory&);

protected:

	void stop();

private:

	void on_failed_to_start(int&) final;
	void on_started() final;

private:

	std::mutex _mutex;
	bool _server_started = false;
	std::condition_variable _server_started_condition;
	std::unique_ptr<ynet::Server> _server;
};
