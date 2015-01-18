#include "connection.h"

namespace ynet
{
	std::string ConnectionImpl::address() const
	{
		return _address._address;
	}

	std::string ConnectionImpl::name() const
	{
		return _address._name;
	}

	uint16_t ConnectionImpl::port() const
	{
		return _address._port;
	}
}
