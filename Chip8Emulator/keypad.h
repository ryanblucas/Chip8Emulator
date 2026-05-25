#pragma once

#include <array>
#include <cassert>
#include "types.h"
#include <vector>
#include <SFML/Window/Keyboard.hpp>

namespace chip8
{
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
		void set_keymap(const std::array<sf::Keyboard::Key, 16>& keys);
		bool is_key_pressed(nibble key) const
		{
			assert(key < 0x10);
			return m_keys[key].pressed;
		}
		const std::array<key, 16>& get_keys() const
		{
			return m_keys;
		}
	private:
		std::array<key, 16> m_keys;
	};

	class key_wait
	{
	public:
		key_wait() : m_reg(0), m_active(false), m_keys({}), m_waiting_keys({}) {}
		bool is_active() const
		{
			return m_active;
		}
		void on_opcode(int reg, const keypad& kp);
		void on_wait(const keypad& kp, std::array<byte, 16>& registers);
	private:
		int m_reg;
		bool m_active;
		std::array<key, 16> m_keys;
		std::vector<int> m_waiting_keys;
	};
}