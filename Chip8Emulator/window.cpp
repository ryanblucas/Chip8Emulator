#include "window.h"
#include "screen.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace chip8;

window::window()
{
	m_window_thread_code = return_code::STILL_RUNNING;
	m_window_thread = std::jthread(&window::thread_proc, this);
	m_screen = screen{};
	m_keypad = keypad{};
}

// i didn't want SFML to be included in window.h, so here's the best solution?
static void window_render_screen(sf::RenderWindow& sfml_window, window& window)
{
	sf::RectangleShape shape{ sf::Vector2f{8, 8} };
	auto back_buffer = window.get_screen().await_get_pixels();

	for (int i = 0; i < screen::HEIGHT; i++)
	{
		for (int j = 0; j < screen::WIDTH; j++)
		{
			if (back_buffer[i * screen::WIDTH + j])
			{
				shape.setPosition(sf::Vector2f(static_cast<float>(j * 8), static_cast<float>(i * 8)));
				sfml_window.draw(shape);
			}
		}
	}
}

static return_code window_handle_events(sf::RenderWindow& sfml_window, window& window)
{
	while (const std::optional<sf::Event> event = sfml_window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			sfml_window.close();
			return return_code::X_BUTTON;
		}
		else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (key_pressed->code == sf::Keyboard::Key::Escape)
			{
				sfml_window.close();
				return return_code::ESCAPE_KEY;
			}
			window.get_keypad().update(true, key_pressed->code);
		}
		else if (const auto* key_released = event->getIf<sf::Event::KeyReleased>())
		{
			window.get_keypad().update(false, key_released->code);
		}
	}
}

void window::thread_proc()
{
	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({ screen::WIDTH * 8, screen::HEIGHT * 8 }), "CHIP-8 Emulator");
	window.setFramerateLimit(144);
	while (window.isOpen())
	{
		m_window_thread_code = window_handle_events(window, *this);
		window.clear();
		window_render_screen(window, *this);
		window.display();
	}
	if (m_window_thread_code == return_code::STILL_RUNNING)
	{
		m_window_thread_code = return_code::UNKNOWN;
	}
}