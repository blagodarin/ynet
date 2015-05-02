#pragma once

#include <cstring>
#include <iomanip>
#include <iostream>

namespace
{
	void dump(const char* data, size_t size)
	{
		static const size_t line_size = 32;
		char buffer[line_size + 1];
		buffer[line_size] = '\0';
		while (size > 0)
		{
			const auto part_size = std::min(size, line_size);
			::memcpy(buffer, data, part_size);
			data += part_size;
			size -= part_size;
			std::cout << std::hex << "\t" << std::setfill('0');
			for (size_t i = 0; i < part_size; ++i)
			{
				const unsigned n = static_cast<unsigned char>(buffer[i]);
				std::cout << std::setw(2) << n << ' ';
				if (n < 32 || n > 127)
					buffer[i] = '.';
			}
			std::cout << std::setfill(' ') << std::setw(2 + (line_size - part_size) * 3) << ' ';
			for (size_t i = 0; i < part_size; ++i)
				std::cout << buffer[i];
			std::cout << std::dec << std::endl;
		}
	}
}
