#include "benchmark.h"

#include <chrono>

namespace
{
	int64_t current_tick()
	{
		 return std::chrono::high_resolution_clock::is_steady
			? std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()
			: std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}
}

BenchmarkClient::BenchmarkClient(const std::string& host, uint16_t port, const ynet::Client::Options& options)
	: _client(ynet::Client::create(*this, host, port, options))
{
}

int64_t BenchmarkClient::run()
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_start_flag = true;
	}
	_start_condition.notify_one();
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_stop_condition.wait(lock, [this]() { return _stop_flag; });
	}
	_client.reset();
	return _discarded ? -1 : _total_ticks;
}

void BenchmarkClient::discard_benchmark()
{
	_discarded = true;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_stop_flag = true;
	}
	_stop_condition.notify_one();
}

void BenchmarkClient::start_benchmark()
{
	_start_tick = ::current_tick();
}

void BenchmarkClient::stop_benchmark()
{
	_total_ticks = ::current_tick() - _start_tick;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_stop_flag = true;
	}
	_stop_condition.notify_one();
}

void BenchmarkClient::on_started(const ynet::Client&)
{
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_start_condition.wait(lock, [this]() { return _start_flag; });
	}
	start_benchmark();
}

BenchmarkServer::BenchmarkServer(uint16_t port)
	: _server(ynet::Server::create(*this, port))
{
	std::unique_lock<std::mutex> lock(_mutex);
	_start_condition.wait(lock, [this]() { return _start_flag; });
}

void BenchmarkServer::on_failed_to_start(const ynet::Server&)
{
}

void BenchmarkServer::on_started(const ynet::Server&)
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_start_flag = true;
	}
	_start_condition.notify_one();
}

void BenchmarkServer::on_stopped(const ynet::Server&)
{
}
