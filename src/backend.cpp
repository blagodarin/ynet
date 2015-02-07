#include "backend.h"

#include "connection.h"

namespace ynet
{
	void ServerHandlers::on_connected(const std::shared_ptr<Connection>& connection)
	{
		_callbacks.on_connected(_server, connection);
	}

	void ServerHandlers::on_received(const std::shared_ptr<Connection>& connection, void* buffer, size_t buffer_size, bool& disconnected)
	{
		for (;;)
		{
			const size_t size = static_cast<ConnectionImpl*>(connection.get())->receive(buffer, buffer_size, &disconnected);
			if (size > 0)
				_callbacks.on_received(_server, connection, buffer, size);
			if (size < buffer_size)
				break;
		}
	}

	void ServerHandlers::on_disconnected(const std::shared_ptr<Connection>& connection)
	{
		_callbacks.on_disconnected(_server, connection);
	}
}
