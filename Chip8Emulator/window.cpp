#include "window.h"
#include <iostream>
#include <numbers>
#include "screen.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace chip8
{
	class beep_timer
	{
	public:
		beep_timer(float frequency)
		{
			m_beep_buffer = create_buffer(frequency);
			m_beep_sound = sf::Sound(m_beep_buffer);
			m_beep_sound->setLooping(true);
		}
		void set(int value)
		{
			if (value > 0 && m_timer_value <= 0)
			{
				m_beep_sound->play();
			}
			m_timer_value = value;
		}
		void tick()
		{
			if (m_timer_value <= 0)
			{
				m_beep_sound->pause();
				return;
			}
			m_timer_value--;
		}
	private:
		// There is no default constructor for sf::Sound and there is also no constructor for just the buffer, only
		// one for a reference to the buffer. Hence, there needs to be an optional
		std::optional<sf::Sound> m_beep_sound;
		sf::SoundBuffer m_beep_buffer;
		int m_timer_value;
		sf::SoundBuffer create_buffer(float frequency)
		{
			const int sample_rate = 44100;
			std::vector<int16_t> samples;
			samples.reserve(sample_rate);
			for (int i = 0; i < sample_rate; i++)
			{
				float t = static_cast<float>(i) / sample_rate;
				float value = std::sin(2.F * static_cast<float>(std::numbers::pi) * frequency * t);
				samples.push_back(static_cast<int16_t>(value * 32767.F));
			}
			std::vector<sf::SoundChannel> channel_map = { sf::SoundChannel::Mono };
			sf::SoundBuffer buffer;
			// idk why but this returns false, yet it throws exceptions when it fails.
			// So... it's kinda fine?
			std::cout << "Creating beep sound buffer returned " << buffer.loadFromSamples(samples.data(), samples.size(), 1, sample_rate, channel_map) << "\n";
			return buffer;
		}
	};
}

using namespace chip8;

window::window(float beep_frequency)
{
	m_window_thread_code = return_code::STILL_RUNNING;
	m_window_thread = std::jthread(&window::thread_proc, this);
	m_screen = screen{};
	m_keypad = keypad{};
	m_beep_timer = std::make_unique<beep_timer>(beep_frequency);
}

window::~window() = default;

void window::beep_set(int ticks)
{
	m_beep_timer->set(ticks);
}

void window::beep_tick()
{
	m_beep_timer->tick();
}

// i didn't want SFML to be included in window.h, so here's the best solution?
static void window_render_screen(sf::RenderWindow& sfml_window, window& window)
{
	sf::RectangleShape shape{ sf::Vector2f{8, 8} };
	auto buffer = window.get_screen().await_get_pixels();

	for (int i = 0; i < screen::HEIGHT; i++)
	{
		for (int j = 0; j < screen::WIDTH; j++)
		{
			if (buffer[i * screen::WIDTH + j])
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
	return return_code::STILL_RUNNING;
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