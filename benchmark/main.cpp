#include <iomanip>
#include <iostream>

#include "connect_disconnect.h"
#include "send_receive.h"

using Row = std::vector<std::string>;
using Table = std::vector<std::vector<std::string>>;

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

void measure_connect_disconnect_rate(Table& results, unsigned count)
{
	std::cout << "Connect-disconnect rate measurement (1 thread, " << count << " ops)..." << std::endl;
	ConnectDisconnectServer server(5445);
	ConnectDisconnectClient client("localhost", 5445, count);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
	{
		results.emplace_back(Row{"FAILED"});
		return;
	}
	const auto seconds = milliseconds / 1000.0;
	results.emplace_back(Row{
		std::to_string(count) + " ops",
		std::to_string(seconds) + " s",
		std::to_string(count / seconds) + " ops/s"});
}

void measure_send_receive_rate(Table& results, unsigned count, size_t bytes)
{
	std::cout << "Local throughput measurement (" << count << "ops, " << bytes << " B)..." << std::endl;
	SendReceiveServer server(5445, bytes);
	SendReceiveClient client("localhost", 5445, count, bytes);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
	{
		results.emplace_back(Row{"FAILED"});
		return;
	}
	const auto seconds = milliseconds / 1000.0;
	results.emplace_back(Row{
		std::to_string(bytes) + " B",
		std::to_string(count) + " ops",
		std::to_string(seconds) + " s",
		std::to_string(count / seconds) + " ops/s",
		std::to_string((2 * static_cast<uint64_t>(count) * bytes) / (seconds * 1024 * 1024)) + " MiB/s"});
}

int main()
{
	{
		Table results;
		measure_connect_disconnect_rate(results, 10000);
		print_table(results);
	}
	{
		Table results;
		for (int i = 0; i <= 22; ++i)
			measure_send_receive_rate(results, 100000, 1 << i);
		print_table(results);
	}
	return 0;
}
