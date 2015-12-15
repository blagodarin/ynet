#include "common.h"
#include "utils.h"

// This should be larger than the connection buffer size (currently 64K).
const size_t BufferSize = 1024 * 1024;

TEST(Local, Send)
{
	const auto& buffer = make_random_buffer(BufferSize);
	SendTestServer server(std::bind(ynet::Server::create_local, std::placeholders::_1, "ynet-tests", std::placeholders::_2), buffer);
	SendTestClient client(std::bind(ynet::Client::create_local, std::placeholders::_1, "ynet-tests", std::placeholders::_2), buffer);
}

TEST(Local, Receive)
{
	const auto& buffer = make_random_buffer(BufferSize);
	ReceiveTestServer server(std::bind(ynet::Server::create_local, std::placeholders::_1, "ynet-tests", std::placeholders::_2), buffer);
	ReceiveTestClient client(std::bind(ynet::Client::create_local, std::placeholders::_1, "ynet-tests", std::placeholders::_2), buffer);
}
