#include "tcp_server.h"

#include <cassert>

namespace ynet
{
	TcpServer::Peer::Peer(TcpBackend::Socket socket, std::string&& address, int port, std::string&& name)
		: _socket(socket)
		, _address(std::move(address))
		, _port(port)
		, _name(std::move(name))
	{
	}

	std::string TcpServer::Peer::address() const
	{
		return _address;
	}

	void TcpServer::Peer::disconnect()
	{
		if (_socket != TcpBackend::InvalidSocket)
		{
			TcpBackend::shutdown(_socket);
			_socket = TcpBackend::InvalidSocket;
		}
	}

	std::string TcpServer::Peer::host() const
	{
		return std::string();
	}

	std::string TcpServer::Peer::name() const
	{
		return _name;
	}

	int TcpServer::Peer::port() const
	{
		return _port;
	}

	bool TcpServer::Peer::send(const void* data, size_t size)
	{
		if (_socket == TcpBackend::InvalidSocket)
			return false;
		auto block = static_cast<const uint8_t*>(data);
		while (size > 0)
		{
			const size_t block_size = std::min(SendBlockSize, size);
			if (!TcpBackend::send(_socket, block, block_size))
				return false;
			block += block_size;
			size -= block_size;
		}
		return true;
	}

	TcpServer::Peer::operator bool() const
	{
		return _socket != TcpBackend::InvalidSocket;
	}

	TcpServer::TcpServer(ServerCallbacks& callbacks, int port)
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
		const auto peer = _peers.emplace(socket, Peer(socket, std::move(address), port, std::move(name))).first;
		_callbacks.on_connected(*this, peer->second);
	}

	void TcpServer::on_received(TcpBackend::Socket socket, bool& disconnected)
	{
		const auto peer = _peers.find(socket);
		assert(peer != _peers.end());
		while (peer->second)
		{
			const size_t size = TcpBackend::recv(socket, _buffer.data(), _buffer.size(), &disconnected);
			if (size > 0)
				_callbacks.on_received(*this, peer->second, _buffer.data(), size);
			if (size < _buffer.size())
				break;
		}
	}

	void TcpServer::on_disconnected(TcpBackend::Socket socket)
	{
		const auto peer = _peers.find(socket);
		assert(peer != _peers.end());
		_callbacks.on_disconnected(*this, peer->second);
		_peers.erase(peer);
	}
}
