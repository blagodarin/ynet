#include "utils.h"

#include <cstdlib>

std::vector<uint8_t> make_random_buffer(size_t size)
{
	std::vector<uint8_t> buffer;
	buffer.reserve(size);
	while (buffer.size() < size)
		buffer.emplace_back(::rand() % UINT8_MAX);
	return std::move(buffer);
}
