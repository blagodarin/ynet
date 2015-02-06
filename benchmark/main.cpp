#include <iomanip>
#include <iostream>
#include <unordered_set>

#include "connect_disconnect.h"
#include "exchange.h"
#include "receive.h"
#include "send.h"

using Row = std::vector<std::string>;
using Table = std::vector<std::vector<std::string>>;

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
}

void benchmark_connect_disconnect(Table& results, unsigned seconds)
{
	std::cout << "Benchmarking connect-disconnect (1 thread, " << seconds << " s)..." << std::endl;
	ConnectDisconnectServer server(5445);
	ConnectDisconnectClient client("localhost", 5445, seconds);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
	{
		results.emplace_back(Row{"FAILED"});
		return;
	}
	const auto marks = client.marks();
	const auto elapsed_seconds = milliseconds / 1000.0;
	results.emplace_back(Row{
		std::to_string(marks) + " ops",
		std::to_string(elapsed_seconds) + " s",
		std::to_string(marks / elapsed_seconds) + " ops/s"});
}

void benchmark_exchange(Table& results, unsigned seconds, size_t bytes)
{
	const auto& human_readable_bytes = ::make_human_readable(bytes);
	std::cout << "Benchmarking exchange (" << seconds << " s, " << human_readable_bytes << ")..." << std::endl;
	ExchangeServer server(5445, bytes);
	ExchangeClient client("localhost", 5445, seconds, bytes);
	const auto elapsed_milliseconds = client.run();
	if (elapsed_milliseconds < 0)
	{
		results.emplace_back(Row{"FAILED"});
		return;
	}
	const auto elapsed_seconds = elapsed_milliseconds / 1000.0;
	const auto marks = client.marks();
	const auto processed_bytes = client.bytes();
	results.emplace_back(Row{
		human_readable_bytes,
		std::to_string(elapsed_seconds) + " s",
		std::to_string(marks) + " ops",
		std::to_string(marks / elapsed_seconds) + " ops/s",
		::make_human_readable(processed_bytes),
		std::to_string(processed_bytes / (elapsed_seconds * 1024 * 1024)) + " MiB/s"});
}

void benchmark_receive(Table& results, unsigned seconds, size_t bytes)
{
	const auto& human_readable_bytes = ::make_human_readable(bytes);
	std::cout << "Benchmarking receive (" << seconds << " s, " << human_readable_bytes << ")..." << std::endl;
	ReceiveServer server(5445, bytes);
	ReceiveClient client("localhost", 5445, seconds, bytes);
	const auto elapsed_milliseconds = client.run();
	if (elapsed_milliseconds < 0)
	{
		results.emplace_back(Row{"FAILED"});
		return;
	}
	const auto elapsed_seconds = elapsed_milliseconds / 1000.0;
	const auto marks = client.marks();
	const auto processed_bytes = client.bytes();
	results.emplace_back(Row{
		human_readable_bytes,
		std::to_string(elapsed_seconds) + " s",
		std::to_string(marks) + " ops",
		std::to_string(marks / elapsed_seconds) + " ops/s",
		::make_human_readable(processed_bytes),
		std::to_string(processed_bytes / (elapsed_seconds * 1024 * 1024)) + " MiB/s"});
}

void benchmark_send(Table& results, unsigned seconds, size_t bytes)
{
	const auto& human_readable_bytes = ::make_human_readable(bytes);
	std::cout << "Benchmarking send (" << seconds << " s, " << human_readable_bytes << ")..." << std::endl;
	SendServer server(5445);
	SendClient client("localhost", 5445, seconds, bytes);
	const auto elapsed_milliseconds = client.run();
	if (elapsed_milliseconds < 0)
	{
		results.emplace_back(Row{"FAILED"});
		return;
	}
	const auto elapsed_seconds = elapsed_milliseconds / 1000.0;
	const auto marks = client.marks();
	const auto processed_bytes = client.bytes();
	results.emplace_back(Row{
		human_readable_bytes,
		std::to_string(elapsed_seconds) + " s",
		std::to_string(marks) + " ops",
		std::to_string(marks / elapsed_seconds) + " ops/s",
		::make_human_readable(processed_bytes),
		std::to_string(processed_bytes / (elapsed_seconds * 1024 * 1024)) + " MiB/s"});
}

int main(int argc, char** argv)
{
	std::unordered_set<std::string> options;
	for (int i = 1; i < argc; ++i)
		options.emplace(argv[i]);
	if (options.count("connect"))
	{
		Table results;
		benchmark_connect_disconnect(results, 1);
		print_table(results);
	}
	if (options.count("send"))
	{
		Table results;
		for (int i = 0; i <= 29; ++i)
			benchmark_send(results, 10, 1 << i);
		print_table(results);
	}
	if (options.count("receive"))
	{
		Table results;
		for (int i = 0; i <= 29; ++i)
			benchmark_receive(results, 10, 1 << i);
		print_table(results);
	}
	if (options.count("exchange"))
	{
		Table results;
		for (int i = 0; i <= 29; ++i)
			benchmark_exchange(results, 10, 1 << i);
		print_table(results);
	}
	return 0;
}
