#pragma once

#include <string>
#include <vector>

namespace ynet
{
	class Address;

	struct Resolved
	{
		std::vector<Address> addresses;
		bool local = false;
	};

	Resolved resolve(const std::string& host, uint16_t port);
}
