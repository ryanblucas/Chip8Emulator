#include "keypad.h"
#include <iostream>

using namespace chip8;

keypad::keypad()
{
	m_keys = {};
	for (int i = 0; i < 10; i++)
	{
		m_keys[i].user_key = static_cast<sf::Keyboard::Key>(static_cast<int>(sf::Keyboard::Key::Num0) + i);
	}
	for (int i = 0; i < 6; i++)
	{
		m_keys[10 + i].user_key = static_cast<sf::Keyboard::Key>(static_cast<int>(sf::Keyboard::Key::A) + i);
	}
}

keypad::keypad(const std::array<key, 16>& keys)
{
	m_keys = keys;
}

void keypad::update(bool is_down, sf::Keyboard::Key key)
{
	for (int i = 0; i < 16; i++)
	{
		if (m_keys[i].user_key == key)
		{
			m_keys[i].pressed = is_down;
			break;
		}
	}
}

void key_wait::on_opcode(int reg, const keypad& kp)
{
	m_reg = reg;
	m_active = true;
	m_keys = kp.get_keys();
}

void key_wait::on_wait(const keypad& kp, std::array<byte, 16>& registers)
{
	for (int i = 0; i < 16; i++)
	{
		if (kp.is_key_pressed(i) != m_keys[i].pressed)
		{
			m_waiting_keys.push_back(i);
		}
	}
	for (int key : m_waiting_keys)
	{
		if (!kp.is_key_pressed(key))
		{
			registers[m_reg] = key;
			m_waiting_keys.clear();
			m_active = false;
			return;
		}
	}
}