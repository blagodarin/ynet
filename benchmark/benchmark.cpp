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

BenchmarkClient::BenchmarkClient(const ClientFactory& factory, int64_t seconds)
	: _factory(factory)
	, _benchmark_time(seconds * 1000)
{
}

int64_t BenchmarkClient::run()
{
	{
		const auto client = _factory(*this);
		client->set_disconnect_timeout(_disconnect_timeout);
		std::unique_lock<std::mutex> lock(_mutex);
		_stop_condition.wait(lock, [this]() { return _stop_flag; });
	}
	return _discarded ? -1 : _elapsed_time;
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
	_start_time = ::current_tick();
}

bool BenchmarkClient::stop_benchmark()
{
	if (!_stopped)
	{
		_elapsed_time = ::current_tick() - _start_time;
		if (_elapsed_time < _benchmark_time)
			return false;
		_stopped = true;
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_stop_flag = true;
		}
		_stop_condition.notify_one();
	}
	return true;
}

void BenchmarkClient::on_started()
{
	start_benchmark();
}

BenchmarkServer::BenchmarkServer(const ServerFactory& factory)
	: _server(factory(*this))
{
	std::unique_lock<std::mutex> lock(_mutex);
	_server_started_condition.wait(lock, [this]() { return _server_started; });
}

void BenchmarkServer::stop()
{
	// This is a workaround to stop _server (and perform all callbacks)
	// before a deriving class starts destroying itself and cleaning up its vtable,
	// resulting in purecall if _server callbacks after that (e.g. on_disconnected).
	// TODO: Remove.
	_server.reset();
}

void BenchmarkServer::on_failed_to_start(int& reconnect_timeout)
{
	reconnect_timeout = 1000;
}

void BenchmarkServer::on_started()
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_server_started = true;
	}
	_server_started_condition.notify_one();
}
