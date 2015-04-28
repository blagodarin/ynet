#pragma once

#include <condition_variable>
#include <mutex>

#include <ynet.h>

enum
{
	BenchmarkLocal = 1 << 0,
};

class BenchmarkClient : public ynet::Client::Callbacks
{
public:

	BenchmarkClient(uint16_t port, int64_t seconds, ynet::Protocol protocol);

	int64_t run();

protected:

	void discard_benchmark();
	void set_disconnect_timeout(int milliseconds) { _disconnect_timeout = milliseconds; }
	void start_benchmark();
	bool stop_benchmark();

private:

	void on_started() final;

private:

	const uint16_t _port;
	const ynet::Protocol _protocol;
	std::mutex _mutex;
	bool _stop_flag = false;
	std::condition_variable _stop_condition;
	int64_t _benchmark_time;
	int64_t _start_time = 0;
	int64_t _elapsed_time = 0;
	bool _stopped = false;
	bool _discarded = false;
	int _disconnect_timeout = 0;
};

class BenchmarkServer : public ynet::Server::Callbacks
{
public:

	BenchmarkServer(uint16_t port, ynet::Protocol protocol);

protected:

	void stop();

private:

	void on_failed_to_start(int&) final;
	void on_started() final;
	void on_stopped() final;

private:

	std::mutex _mutex;
	bool _server_started = false;
	std::condition_variable _server_started_condition;
	std::unique_ptr<ynet::Server> _server;
};
