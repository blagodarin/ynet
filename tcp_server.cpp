#include "tcp_server.h"

#include <cassert>

namespace ynet
{
	TcpServer::TcpServer(Server::Callbacks& callbacks, int port)
		: _callbacks(callbacks)
		, _port(port >= 0 && port <= 65535 ? port : -1)
		, _socket(TcpBackend::InvalidSocket)
		, _poller(*this)
	{
		if (_port < 0)
			return; // TODO: Throw.
		_socket = TcpBackend::listen(_port, _address, _name);
		if (_socket == TcpBackend::InvalidSocket)
			return; // TODO: Retry.
		_thread = std::thread(std::bind(&TcpServer::run, this));
	}

	TcpServer::~TcpServer()
	{
		if (_socket == TcpBackend::InvalidSocket)
			return;
		TcpBackend::shutdown_server(_socket);
		_thread.join();
		TcpBackend::close(_socket);
	}

	std::string TcpServer::address() const
	{
		return _address;
	}

	std::string TcpServer::name() const
	{
		return _name;
	}

	int TcpServer::port() const
	{
		return _port;
	}

	void TcpServer::run()
	{
		_callbacks.on_started(*this);
		_poller.run(_socket);
		_callbacks.on_stopped(*this);
	}

	void TcpServer::on_connected(TcpBackend::Socket socket, std::string&& address, int port, std::string&& name)
	{
		const auto& connection = std::make_shared<TcpConnection>(socket, std::move(address), port, std::move(name));
		_connections.emplace(socket, connection);
		_callbacks.on_connected(*this, connection);
	}

	void TcpServer::on_received(TcpBackend::Socket socket, bool& disconnected)
	{
		const auto connection = _connections.find(socket);
		assert(connection != _connections.end());
		while (*connection->second)
		{
			const size_t size = TcpBackend::recv(socket, _buffer.data(), _buffer.size(), &disconnected);
			if (size > 0)
				_callbacks.on_received(*this, connection->second, _buffer.data(), size);
			if (size < _buffer.size())
				break;
		}
	}

	void TcpServer::on_disconnected(TcpBackend::Socket socket)
	{
		const auto connection = _connections.find(socket);
		assert(connection != _connections.end());
		_callbacks.on_disconnected(*this, connection->second);
		_connections.erase(connection);
	}
}
