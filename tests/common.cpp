#include "common.h"

void TestClient::start(const Factory& factory)
{
	_client = factory(*this);
	_client->set_shutdown_timeout(-1);
}

void TestClient::stop()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_stop_condition.wait(lock, [this]() { return _stopped; });
}

void TestClient::on_started()
{
}

void TestClient::on_failed_to_connect(int&)
{
	ADD_FAILURE();
}

void TestClient::on_stopped()
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_stopped = true;
	}
	_stop_condition.notify_one();
}

void TestServer::start(const Factory& factory)
{
	_server = factory(*this);
	std::unique_lock<std::mutex> lock(_mutex);
	_start_condition.wait(lock, [this]() { return _started; });
}

void TestServer::stop()
{
	_server.reset();
}

void TestServer::on_failed_to_start(int& restart_timeout)
{
	restart_timeout = 1000;
	// TODO: Limit the startup by an amount of time or a number of attempts, failing the test if unable to start.
}

void TestServer::on_started()
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_started = true;
	}
	_start_condition.notify_one();
}

SendTestClient::SendTestClient(const Factory& factory, const std::vector<uint8_t>& buffer)
	: _buffer(buffer)
{
	start(factory);
}

SendTestClient::~SendTestClient()
{
	stop();
}

void SendTestClient::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	EXPECT_TRUE(connection->send(_buffer.data(), _buffer.size()));
}

void SendTestClient::on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
	ADD_FAILURE();
}

void SendTestClient::on_disconnected(const std::shared_ptr<ynet::Connection>&, int&)
{
}

SendTestServer::SendTestServer(const Factory& factory, const std::vector<uint8_t>& buffer)
	: _buffer(buffer)
	, _received(buffer.size())
{
	start(factory);
}

SendTestServer::~SendTestServer()
{
	stop();
}

void SendTestServer::on_connected(const std::shared_ptr<ynet::Connection>&)
{
	ASSERT_EQ(_received_size, 0);
}

void SendTestServer::on_received(const std::shared_ptr<ynet::Connection>& connection, const void* data, size_t size)
{
	const auto remaining_size = _received.size() - _received_size;
	ASSERT_GE(remaining_size, size);
	::memcpy(&_received[_received_size], static_cast<const uint8_t*>(data), size);
	_received_size += size;
	if (_received_size == _received.size())
		connection->close();
}

void SendTestServer::on_disconnected(const std::shared_ptr<ynet::Connection>&)
{
	EXPECT_EQ(_received, _buffer);
}

ReceiveTestClient::ReceiveTestClient(const Factory& factory, const std::vector<uint8_t>& buffer)
	: _buffer(buffer)
	, _received(buffer.size())
{
	start(factory);
}

ReceiveTestClient::~ReceiveTestClient()
{
	stop();
}

void ReceiveTestClient::on_connected(const std::shared_ptr<ynet::Connection>&)
{
	ASSERT_EQ(_received_size, 0);
}

void ReceiveTestClient::on_received(const std::shared_ptr<ynet::Connection>&, const void* data, size_t size)
{
	const auto remaining_size = _received.size() - _received_size;
	ASSERT_GE(remaining_size, size);
	::memcpy(&_received[_received_size], static_cast<const uint8_t*>(data), size);
	_received_size += size;
}

void ReceiveTestClient::on_disconnected(const std::shared_ptr<ynet::Connection>&, int&)
{
	EXPECT_EQ(_received, _buffer);
}

ReceiveTestServer::ReceiveTestServer(const Factory& factory, const std::vector<uint8_t>& buffer)
	: _buffer(buffer)
{
	start(factory);
}

ReceiveTestServer::~ReceiveTestServer()
{
	stop();
}

void ReceiveTestServer::on_connected(const std::shared_ptr<ynet::Connection>& connection)
{
	EXPECT_TRUE(connection->send(_buffer.data(), _buffer.size()));
	connection->close();
}

void ReceiveTestServer::on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t)
{
	ADD_FAILURE();
}

void ReceiveTestServer::on_disconnected(const std::shared_ptr<ynet::Connection>&)
{
}
