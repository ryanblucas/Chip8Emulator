#include "screen.h"
#include <algorithm>

using namespace chip8;

sprite::sprite(const std::vector<byte>& data, int pos, memory* memory)
{
	assert(data.size() >= 1 && data.size() <= 15 && pos >= 0 && pos + data.size() <= memory::CAPACITY && memory);
	m_size = static_cast<int>(data.size());
	m_data_pos = pos;
	m_memory = memory;
	for (int i = 0; i < m_size; i++)
	{
		m_memory->set_byte(m_data_pos + i, data[i]);
	}
}

sprite::sprite(int size, int pos, memory* memory)
{
	assert(size >= 1 && size <= 15 && pos >= 0 && pos + size <= memory::CAPACITY && memory);
	m_size = size;
	m_data_pos = pos;
	m_memory = memory;
}

screen::screen()
{
	m_pixels = {};
	m_lock_pixels = false;
}

void screen::clear()
{
	m_pixels.fill(false);
}

void screen::draw_sprite(int x, int y, const sprite& spr)
{
	// spec says only stipulation is that x and y be within the bounds of the screen,
	// nothing about clipping part of a sprite if the bottom or right edge goes out of bounds
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
	{
		return;
	}
	m_lock_pixels = true;
	for (int i = 0; i < std::min(HEIGHT, y + spr.get_height()) - y; i++)
	{
		byte row = spr.get_row(i);
		byte bitmask = byte{ 0x80 };
		for (int j = 0; j < std::min(WIDTH, x + 8) - x; j++)
		{
			m_pixels[(y + i) * WIDTH + x + j] ^= (row & bitmask) != byte{ 0 };
			bitmask >>= 1;
		}
	}
	m_lock_pixels = false;
}