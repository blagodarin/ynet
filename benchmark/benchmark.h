#pragma once

#include <condition_variable>
#include <mutex>

#include <ynet.h>

class BenchmarkClient: public ynet::Client::Callbacks
{
public:

	BenchmarkClient(const std::string& host, uint16_t port, int64_t seconds, const ynet::Client::Options& options = {});

	int64_t run();

protected:

	void discard_benchmark();
	void start_benchmark();
	bool stop_benchmark();

private:

	void on_started(const ynet::Client&) final;

private:

	std::mutex _mutex;
	bool _start_flag = false;
	std::condition_variable _start_condition;
	bool _stop_flag = false;
	std::condition_variable _stop_condition;
	int64_t _benchmark_time;
	int64_t _start_time = 0;
	int64_t _elapsed_time = 0;
	bool _stopped = false;
	bool _discarded = false;
	std::unique_ptr<ynet::Client> _client;
};

class BenchmarkServer: public ynet::Server::Callbacks
{
public:

	BenchmarkServer(uint16_t port);

private:

	void on_failed_to_start(const ynet::Server&) final;
	void on_started(const ynet::Server&) final;
	void on_stopped(const ynet::Server&) final;

private:

	std::mutex _mutex;
	bool _start_flag = false;
	std::condition_variable _start_condition;
	std::unique_ptr<ynet::Server> _server;
};
