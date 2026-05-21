#pragma once

#include <array>
#include <cassert>
#include <memory>
#include "memory.h"
#include "types.h"
#include <vector>

namespace chip8
{
	class sprite
	{
	public:
		// must be from 1-15 bytes long
		sprite(const std::vector<byte>& data, int pos, memory* memory);
		// must be from 1-15 bytes long
		sprite(int size, int pos, memory* memory);
		sprite(nibble character, memory* memory) : sprite(5, memory::PROGRAM_FONT_START + character * 5, memory) {}

		byte get_row(int y) const
		{
			assert(y < m_size && y >= 0);
			return m_memory->get_byte(m_data_pos + y);
		}
		bool get_pixel(int x, int y) const
		{
			assert(x >= 0 && x <= 7);
			return ((get_row(y) >> (7 - x)) & byte { 0x01 }) == byte{ 0x01 };
		}
		int get_height() const
		{
			return m_size;
		}
	private:
		int m_size;
		int m_data_pos;
		memory* m_memory;
	};

	class window;
	class screen
	{
	public:
		static const int WIDTH = 64;
		static const int HEIGHT = 32;

		screen();
		void clear();
		void draw_sprite(int x, int y, const sprite& spr);

		// gets pixels if sprite is not being drawn, otherwise waits until that is completed to return pixels
		std::span<const bool> await_get_pixels() const
		{
			while (m_lock_pixels);
			return std::span<const bool>(m_pixels);
		}
	private:
		std::array<bool, WIDTH * HEIGHT> m_pixels;
		volatile bool m_lock_pixels;
	};
}