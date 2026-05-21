#pragma once

#include <array>
#include <cassert>
#include "types.h"
#include <vector>
#include <SFML/Window/Keyboard.hpp>

namespace chip8
{
	class window;
	struct key
	{
		sf::Keyboard::Key user_key;
		bool pressed;
	};
	class keypad
	{
	public:
		keypad();
		keypad(const std::array<key, 16>& keys);
		// user keys are the keys the user has currently pressed down from their keyboard.
		// this function maps those to the corresponding CHIP-8 keys and updates the state of the keys in the emulator.
		void update(bool is_down, sf::Keyboard::Key key);
		bool is_key_pressed(nibble key) const
		{
			assert(key < 0x10);
			return m_keys[key].pressed;
		}
		void set_keys(const std::array<key, 16>& keys)
		{
			m_keys = keys;
		}
	private:
		std::array<key, 16> m_keys;
	};
}