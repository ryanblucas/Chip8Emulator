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

	class beep_timer;
	class window
	{
	public:
		window(float beep_frequency = 440.F);
		~window();
		void beep_set(int ticks);
		void beep_tick();
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
		static const int BEEP_FREQUENCY = 350;
		screen m_screen;
		keypad m_keypad;
		std::jthread m_window_thread;
		return_code m_window_thread_code;

		std::unique_ptr<beep_timer> m_beep_timer;

		void thread_proc();
	};
}