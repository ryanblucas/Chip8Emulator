#pragma once

#include "keypad.h"
#include "screen.h"
#include <thread>

namespace chip8
{
	class window
	{
	public:
		window();
		screen& get_screen()
		{
			return m_screen;
		}
		keypad& get_keypad()
		{
			return m_keypad;
		}
		std::jthread& get_thread()
		{
			return m_window_thread;
		}
		bool is_open() const
		{
			return m_window_thread_open;
		}
	private:
		screen m_screen;
		keypad m_keypad;
		std::jthread m_window_thread;
		bool m_window_thread_open;

		void thread_proc();
	};
}