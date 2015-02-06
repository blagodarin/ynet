#pragma once

#include "connection.h"

namespace ynet
{
	std::unique_ptr<ConnectionImpl> create_local_connection(uint16_t port);
}
