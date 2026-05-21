#pragma once

#include "keypad.h"
#include "screen.h"
#include <thread>

namespace chip8
{
	enum class return_code
	{
		STILL_RUNNING,
		X_BUTTON,
		ESCAPE_KEY,
		UNKNOWN,
	};

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
		return_code get_return_code() const
		{
			return m_window_thread_code;
		}
	private:
		screen m_screen;
		keypad m_keypad;
		std::jthread m_window_thread;
		return_code m_window_thread_code;

		void thread_proc();
	};
}