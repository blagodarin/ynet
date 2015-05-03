#include "common.h"
#include "utils.h"

// This should be larger than the connection buffer size (currently 64K).
const size_t BufferSize = 1024 * 1024;

TEST(Tcp, Send)
{
	const auto& buffer = make_random_buffer(BufferSize);
	SendTestServer server(std::bind(ynet::Server::create_tcp, std::placeholders::_1, 20000), buffer);
	SendTestClient client(std::bind(ynet::Client::create_tcp, std::placeholders::_1, "localhost", 20000), buffer);
}

TEST(Tcp, Receive)
{
	const auto& buffer = make_random_buffer(BufferSize);
	ReceiveTestServer server(std::bind(ynet::Server::create_tcp, std::placeholders::_1, 20001), buffer);
	ReceiveTestClient client(std::bind(ynet::Client::create_tcp, std::placeholders::_1, "localhost", 20001), buffer);
}
