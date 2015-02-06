#pragma once

#include <ynet.h>
#include "address.h"

namespace ynet
{
	class ConnectionImpl: public Connection
	{
	public:

		ConnectionImpl(const ::sockaddr_storage& sockaddr): _address(sockaddr) {}
		ConnectionImpl(uint16_t port): _address(port) {}
		~ConnectionImpl() override = default;

		std::string address() const override;
		std::string name() const override;
		uint16_t port() const override;

		virtual size_t receive(void* data, size_t size, bool* disconnected) = 0;
		virtual size_t receive_buffer_size() const = 0;

	private:

		const Address _address;
	};
}
