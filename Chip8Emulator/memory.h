#pragma once

#include <array>
#include <string>
#include <span>
#include "types.h"

namespace chip8
{
	union memory_instruction
	{
		uint16_t raw;
		struct
		{
			nibble s4 : 4; // 4th most significant nibble
			nibble s3 : 4;
			nibble s2 : 4;
			nibble s1 : 4; // 1st most significant nibble
		} nibbles;
		struct
		{
			byte b2; // 2nd most significant byte
			byte b1; // 1st most significant byte
		} bytes;
	};

	class memory
	{
	public:
		// the number chosen is largely arbritary. I wanted to reserve the first 0x80 bytes for
		// null pointers, but does that even matter for CHIP-8?
		static const address PROGRAM_FONT_START = 0x080;
		static const address PROGRAM_START = 0x200;
		static const int CAPACITY = 0x1000;
		static const int USER_CAPACITY = CAPACITY - PROGRAM_START;

		memory();
		memory(std::string_view program_path);

		// operator[] is not used here because the range check for getting
		// a byte is different than the range check for setting a byte
		byte get_byte(address addr) const;
		void set_byte(address addr, byte byte);
		memory_instruction get_instruction(address addr) const;
	private:
		std::array<byte, CAPACITY> m_memory;
		
		void load_file_to(std::string_view path, std::streamsize min_size, std::streamsize max_size, address start);
	};
}