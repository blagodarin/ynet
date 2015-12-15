#pragma once

#include <condition_variable>

#include <gtest/gtest.h>

#include <ynet.h>

class TestClient : public ynet::Client::Callbacks
{
public:
	using Factory = std::function<std::unique_ptr<ynet::Client>(ynet::Client::Callbacks&, const ynet::Client::Options&)>;

protected:
	void start(const Factory&);
	void stop();

private:
	void on_started() final;
	void on_failed_to_connect(int&) final;
	void on_stopped() final;

private:
	std::mutex _mutex;
	bool _stopped = false;
	std::condition_variable _stop_condition;
	std::unique_ptr<ynet::Client> _client;
};

class TestServer : public ynet::Server::Callbacks
{
public:
	using Factory = std::function<std::unique_ptr<ynet::Server>(ynet::Server::Callbacks&, const ynet::Server::Options&)>;

protected:
	void start(const Factory&);
	void stop();

private:
	void on_failed_to_start(int&) final;
	void on_started() final;

private:
	std::mutex _mutex;
	bool _started = false;
	std::condition_variable _start_condition;
	std::unique_ptr<ynet::Server> _server;
};

class SendTestClient : public TestClient
{
public:
	SendTestClient(const Factory& factory, const std::vector<uint8_t>& buffer);
	~SendTestClient() override;

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&, int&) override;

private:
	const std::vector<uint8_t>& _buffer;
};

class SendTestServer : public TestServer
{
public:
	SendTestServer(const Factory& factory, const std::vector<uint8_t>& buffer);
	~SendTestServer() override;

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&) override;

private:
	const std::vector<uint8_t>& _buffer;
	std::vector<uint8_t> _received;
	size_t _received_size = 0;
};

class ReceiveTestClient : public TestClient
{
public:
	ReceiveTestClient(const Factory& factory, const std::vector<uint8_t>& buffer);
	~ReceiveTestClient() override;

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&, int&) override;

private:
	const std::vector<uint8_t>& _buffer;
	std::vector<uint8_t> _received;
	size_t _received_size = 0;
};

class ReceiveTestServer : public TestServer
{
public:
	ReceiveTestServer(const Factory& factory, const std::vector<uint8_t>& buffer);
	~ReceiveTestServer() override;

private:
	void on_connected(const std::shared_ptr<ynet::Connection>&) override;
	void on_received(const std::shared_ptr<ynet::Connection>&, const void*, size_t) override;
	void on_disconnected(const std::shared_ptr<ynet::Connection>&) override;

private:
	const std::vector<uint8_t>& _buffer;
};
