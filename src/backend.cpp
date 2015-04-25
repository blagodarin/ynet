#include "backend.h"

#include "connection.h"

namespace ynet
{
	void ServerBackend::Callbacks::on_received(const std::shared_ptr<Connection>& connection, void* buffer, size_t buffer_size, bool& disconnected)
	{
		for (;;)
		{
			const size_t size = static_cast<ConnectionImpl*>(connection.get())->receive(buffer, buffer_size, &disconnected);
			if (size > 0)
				_callbacks.on_received(connection, buffer, size);
			if (size < buffer_size)
				break;
		}
	}
}
