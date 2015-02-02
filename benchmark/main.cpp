#include <iostream>

#include "connect_disconnect.h"
#include "send_receive.h"

void measure_connect_disconnect_rate(unsigned attempts)
{
	ConnectDisconnectServer server(5445);
	ConnectDisconnectClient client("localhost", 5445, attempts);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
	{
		std::cout << "\tFAILED" << std::endl;
		return;
	}
	const auto seconds = milliseconds / 1000.0;
	std::cout << "\t" << attempts << " / " << seconds << " CD/s = " << attempts / seconds << " CD/s" << std::endl;
}

void measure_send_receive_rate(unsigned cycles, size_t size)
{
	SendReceiveServer server(5445, size);
	SendReceiveClient client("localhost", 5445, cycles, size);
	const auto milliseconds = client.run();
	if (milliseconds < 0)
	{
		std::cout << "\tFAILED" << std::endl;
		return;
	}
	const auto seconds = milliseconds / 1000.0;
	std::cout << "\t" << size << " B: " << cycles << " / " << seconds << " SR/s = " << cycles / seconds << " SR/s" << std::endl;
}

int main()
{
	std::cout << "Connect-disconnect rate measurement (single-threaded)..." << std::endl;
	measure_connect_disconnect_rate(10000);
	std::cout << std::endl;

	std::cout << "Send-receive rate measurement..." << std::endl;
	for (int i = 0; i < 21; ++i)
		measure_send_receive_rate(100000, 1 << i);
	std::cout << std::endl;

	return 0;
}
