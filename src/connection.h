#pragma once

#include <ynet.h>

namespace ynet
{
	class ConnectionImpl : public Connection
	{
	public:

		ConnectionImpl(std::string&& address): _address(std::move(address)) {}
		~ConnectionImpl() override = default;

		std::string address() const override { return _address; }

		virtual size_t receive(void* data, size_t size, bool* disconnected) = 0;
		virtual size_t receive_buffer_size() const = 0;

	private:

		const std::string _address;
	};
}
