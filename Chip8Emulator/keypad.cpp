#include "keypad.h"

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