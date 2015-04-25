#pragma once

#include <ynet.h>
#include "address.h"

namespace ynet
{
	class ConnectionImpl : public Connection
	{
	public:

		ConnectionImpl(const Address& address): _address(address) {}
		~ConnectionImpl() override = default;

		std::string address() const override { return _address.ip(); }

		virtual size_t receive(void* data, size_t size, bool* disconnected) = 0;
		virtual size_t receive_buffer_size() const = 0;

	private:

		const Address _address;
	};
}
