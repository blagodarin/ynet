#include "local.h"

#include <cstddef>

#include <sys/un.h>

#include "socket.h"

namespace ynet
{
	const size_t LocalBufferSize = 64 * 1024; // Arbitrary value.

	std::unique_ptr<ConnectionImpl> create_local_connection(uint16_t port)
	{
		::sockaddr_un sockaddr;
		::memset(&sockaddr, 0, sizeof sockaddr);
		sockaddr.sun_family = AF_UNIX;
		char* cursor = sockaddr.sun_path;
		*cursor++ = '\0'; // Linux-specific abstact socket.
		*cursor++ = 'y';
		*cursor++ = 'n';
		*cursor++ = 'e';
		*cursor++ = 't';
		*cursor++ = '.';
		::sprintf(cursor, "%d", port);
		const size_t sockaddr_size = offsetof(sockaddr_un, sun_path) + (cursor - sockaddr.sun_path) + ::strlen(cursor) + 1;
		Socket socket = ::socket(sockaddr.sun_family, SOCK_STREAM, 0);
		if (!socket)
			return {};
		if (::connect(socket.get(), reinterpret_cast<const ::sockaddr*>(&sockaddr), sockaddr_size) == -1)
			return {};
		return std::unique_ptr<ConnectionImpl>(new SocketConnection(port, std::move(socket), 0, LocalBufferSize));
	}
}
