#pragma once

#include <cstddef>
#include <cstdint>

namespace chip8
{
	using nibble = uint16_t;
	// std::byte does not allow for adding and subtracting like uint8_t does
	using byte = uint8_t;
	using address = uint16_t;
}