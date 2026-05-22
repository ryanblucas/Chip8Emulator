#include "memory.h"
#include <cassert>
#include <stdexcept>
#include <format>
#include <fstream>
#include <cstring>

using namespace chip8;

memory::memory(std::string_view program_path)
{
	m_memory = {};
	load_file_to(program_path, 0, USER_CAPACITY, PROGRAM_START);
	load_file_to("font.ch8", 5 * 0x10, 5 * 0x10, PROGRAM_FONT_START);
}

memory::memory()
{
	m_memory = {};
	load_file_to("font.ch8", 5 * 0x10, 5 * 0x10, PROGRAM_FONT_START);
}

void memory::load_file_to(std::string_view path, std::streamsize min_size, std::streamsize max_size, address start)
{
	std::ifstream program_file(path.data(), std::ios::binary);
	if (!program_file)
	{
		throw std::runtime_error("Failed to open file: " + std::string(std::strerror(errno)));
	}
	program_file.seekg(0, std::ios::end);
	std::streamsize size = program_file.tellg();
	program_file.seekg(0, std::ios::beg);
	if (size > max_size || size < min_size)
	{
		throw std::runtime_error(std::format("File (\"{}\") size is incorrect", path));
	}
	program_file.read(reinterpret_cast<char*>(m_memory.data() + start), size);
}

byte memory::get_byte(address addr) const
{
	if (addr >= m_memory.size())
	{
		throw std::runtime_error("Cannot get a byte outside of range");
	}
	return m_memory[addr];
}

void memory::set_byte(address addr, byte byte)
{
	if (addr >= m_memory.size() || addr < PROGRAM_START)
	{
		throw std::runtime_error("Cannot set a byte outside of user range");
	}
	m_memory[addr] = byte;
}

memory_instruction memory::get_instruction(address addr) const
{
	if ((addr & 1) != 0)
	{
		throw std::runtime_error("Tried to get the instruction at an odd address");
	}
	// big-endian
	return memory_instruction{ .raw = static_cast<uint16_t>(get_byte(addr) << 8 | get_byte(addr + 1)) };
}