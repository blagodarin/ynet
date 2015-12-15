#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <unordered_set>

#include "connect_disconnect.h"
#include "exchange.h"
#include "receive.h"
#include "send.h"

using Row = std::vector<std::string>;
using Table = std::vector<std::vector<std::string>>;

struct BenchmarkResults
{
	uint64_t milliseconds = 0;
	uint64_t operations = 0;
	size_t unit_bytes = 0;
	uint64_t total_bytes = 0;

	BenchmarkResults() = default;

	BenchmarkResults(uint64_t milliseconds, uint64_t operations)
		: milliseconds(milliseconds)
		, operations(operations)
	{
	}

	BenchmarkResults(uint64_t milliseconds, uint64_t operations, size_t unit_bytes, uint64_t total_bytes)
		: milliseconds(milliseconds)
		, operations(operations)
		, unit_bytes(unit_bytes)
		, total_bytes(total_bytes)
	{
	}
};

namespace
{
	template <class T>
	std::string make_human_readable(T bytes)
	{
		if (bytes < 1024)
			return std::to_string(bytes) + " B";
		bytes /= 1024;
		if (bytes < 1024)
			return std::to_string(bytes) + " K";
		bytes /= 1024;
		if (bytes < 1024)
			return std::to_string(bytes) + " M";
		bytes /= 1024;
		if (bytes < 1024)
			return std::to_string(bytes) + " G";
		return std::to_string(bytes) + " T";
	}

	void print_table(const Table& table)
	{
		size_t max_row_size = 0;
		for (const auto& row : table)
			max_row_size = std::max(max_row_size, row.size());

		std::vector<size_t> widths(max_row_size);
		for (const auto& row : table)
			for (size_t i = 0; i < row.size(); ++i)
				widths[i] = std::max(widths[i], row[i].size());

		std::cout << std::endl;
		for (const auto& row : table)
		{
			for (size_t i = 0; i < row.size(); ++i)
				std::cout << (i ? "   " : "\t") << std::setw(widths[i]) << row[i];
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	void print_results(const std::vector<BenchmarkResults>& results)
	{
		Table table;
		table.reserve(results.size());
		for (const auto& result : results)
		{
			Row row;
			if (result.unit_bytes > 0)
				row.emplace_back(make_human_readable(result.unit_bytes));
			const auto seconds = result.milliseconds / 1000.0;
			row.emplace_back(std::to_string(seconds) + " s");
			row.emplace_back(std::to_string(result.operations) + " ops");
			row.emplace_back(std::to_string(result.operations / seconds) + " ops/s");
			if (result.unit_bytes > 0)
			{
				row.emplace_back(make_human_readable(result.total_bytes));
				row.emplace_back(std::to_string(result.total_bytes / (seconds * 1024 * 1024)) + " MiB/s");
			}
			table.emplace_back(std::move(row));
		}
		print_table(table);
	}

	void print_compared(const std::vector<BenchmarkResults>& first, const std::vector<BenchmarkResults>& second)
	{
		Table table;
		assert(first.size() == second.size());
		table.reserve(first.size());
		for (size_t i = 0; i < first.size(); ++i)
		{
			Row row;
			assert(first[i].unit_bytes == second[i].unit_bytes);
			if (first[i].unit_bytes > 0)
				row.emplace_back(make_human_readable(first[i].unit_bytes));
			const auto seconds = first[i].milliseconds / 1000.0;
			row.emplace_back(std::to_string(seconds) + " s");
			const auto first_ops_s = ::lround(first[i].operations / seconds);
			row.emplace_back(std::to_string(first_ops_s) + " ops/s");
			const auto second_ops_s = ::lround(second[i].operations / seconds);
			row.emplace_back(std::to_string(second_ops_s) + " ops/s");
			if (first[i].unit_bytes > 0)
			{
				row.emplace_back(std::to_string(first[i].total_bytes / (seconds * 1024 * 1024)) + " MiB/s");
				row.emplace_back(std::to_string(second[i].total_bytes / (seconds * 1024 * 1024)) + " MiB/s");
			}
			row.emplace_back(std::to_string(second_ops_s * 1.0 / first_ops_s) + " x");
			table.emplace_back(std::move(row));
		}
		print_table(table);
	}
}

template <class Factory>
BenchmarkResults benchmark_connect_disconnect(unsigned seconds)
{
	std::cout << "Benchmarking connect-disconnect (1 thread, " << seconds << " s)..." << std::endl;
	ConnectDisconnectServer server(Factory::create_server);
	ConnectDisconnectClient client(Factory::create_client, seconds);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
		return {};
	return BenchmarkResults(milliseconds, client.marks());
}

template <class Factory>
BenchmarkResults benchmark_exchange(unsigned seconds, size_t bytes)
{
	const auto& human_readable_bytes = ::make_human_readable(bytes);
	std::cout << "Benchmarking exchange (" << seconds << " s, " << human_readable_bytes << ")..." << std::endl;
	ExchangeServer server(Factory::create_server, bytes);
	ExchangeClient client(Factory::create_client, seconds, bytes);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
		return {};
	return BenchmarkResults(milliseconds, client.marks(), bytes, client.bytes());
}

template <class Factory>
BenchmarkResults benchmark_receive(unsigned seconds, size_t bytes)
{
	const auto& human_readable_bytes = ::make_human_readable(bytes);
	std::cout << "Benchmarking receive (" << seconds << " s, " << human_readable_bytes << ")..." << std::endl;
	ReceiveServer server(Factory::create_server, bytes);
	ReceiveClient client(Factory::create_client, seconds, bytes);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
		return {};
	return BenchmarkResults(milliseconds, client.marks(), bytes, client.bytes());
}

template <class Factory>
BenchmarkResults benchmark_send(unsigned seconds, size_t bytes)
{
	const auto& human_readable_bytes = ::make_human_readable(bytes);
	std::cout << "Benchmarking send (" << seconds << " s, " << human_readable_bytes << ")..." << std::endl;
	SendServer server(Factory::create_server);
	SendClient client(Factory::create_client, seconds, bytes);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
		return {};
	return BenchmarkResults(milliseconds, client.marks(), bytes, client.bytes());
}

int main(int argc, char** argv)
{
	std::unordered_set<std::string> options;
	for (int i = 1; i < argc; ++i)
		options.emplace(argv[i]);
	const int test_seconds = options.count("quick") ? 1 : 10;
	if (options.count("connect"))
	{
		std::vector<BenchmarkResults> results;
		results.emplace_back(benchmark_connect_disconnect<BenchmarkTcp>(1)); // TODO: Change seconds to attempts.
		print_results(results);
	}
	if (options.count("connect-local"))
	{
		std::vector<BenchmarkResults> results;
		results.emplace_back(benchmark_connect_disconnect<BenchmarkLocal>(test_seconds));
		print_results(results);
	}
	if (options.count("send"))
	{
		std::vector<BenchmarkResults> results;
		for (int i = 0; i <= 29; ++i)
			results.emplace_back(benchmark_send<BenchmarkTcp>(test_seconds, 1 << i));
		print_results(results);
	}
	if (options.count("receive"))
	{
		std::vector<BenchmarkResults> results;
		for (int i = 0; i <= 29; ++i)
			results.emplace_back(benchmark_receive<BenchmarkTcp>(test_seconds, 1 << i));
		print_results(results);
	}
	if (options.count("exchange"))
	{
		std::vector<BenchmarkResults> results;
		for (int i = 0; i <= 29; ++i)
			results.emplace_back(benchmark_exchange<BenchmarkTcp>(test_seconds, 1 << i));
		print_results(results);
	}
	if (options.count("local"))
	{
		std::vector<BenchmarkResults> tcp;
		std::vector<BenchmarkResults> local;
		for (int i = 0; i <= 29; ++i)
		{
			tcp.emplace_back(benchmark_send<BenchmarkTcp>(test_seconds, 1 << i));
			local.emplace_back(benchmark_send<BenchmarkLocal>(test_seconds, 1 << i));
		}
		print_compared(tcp, local);
		tcp.clear();
		local.clear();
		for (int i = 0; i <= 29; ++i)
		{
			tcp.emplace_back(benchmark_receive<BenchmarkTcp>(test_seconds, 1 << i));
			local.emplace_back(benchmark_receive<BenchmarkLocal>(test_seconds, 1 << i));
		}
		print_compared(tcp, local);
		tcp.clear();
		local.clear();
		for (int i = 0; i <= 29; ++i)
		{
			tcp.emplace_back(benchmark_exchange<BenchmarkTcp>(test_seconds, 1 << i));
			local.emplace_back(benchmark_exchange<BenchmarkLocal>(test_seconds, 1 << i));
		}
		print_compared(tcp, local);
	}
	return 0;
}
