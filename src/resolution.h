#pragma once

#include <string>
#include <vector>

namespace ynet
{
	class Address;

	class Resolution
	{
	public:

		Resolution(const std::string& host, uint16_t port);

		std::vector<Address>::const_iterator begin() const { return _addresses.begin(); }
		std::vector<Address>::const_iterator end() const { return _addresses.end(); }
		bool local() const { return _local; }

	private:

		std::vector<Address> _addresses;
		bool _local = false;
	};
}
