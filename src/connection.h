#pragma once

#include <ynet.h>
#include "address.h"

namespace ynet
{
	enum class ConnectionSide
	{
		Client,
		Server,
	};

	class ConnectionImpl : public Connection
	{
	public:

		ConnectionImpl(const ::sockaddr_storage& sockaddr): _address(sockaddr) {}
		~ConnectionImpl() override = default;

		std::string address() const override;

		virtual size_t receive(void* data, size_t size, bool* disconnected) = 0;
		virtual size_t receive_buffer_size() const = 0;

	private:

		const Address _address;
	};
}
