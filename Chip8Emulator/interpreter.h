#pragma once

#include <chrono>
#include <stack>
#include "window.h"

namespace chip8
{
	enum opcode : uint16_t
	{
		CLEAR_DISPLAY = 0x00E0,
		RETURN = 0x00EE,
		JUMP = 0x1000,
		CALL = 0x2000,
		SKIP_IFEQK = 0x3000,
		SKIP_IFNEQK = 0x4000,
		SKIP_IFEQV = 0x5000,
		ASSIGNK = 0x6000,
		ADDK = 0x7000,
		ASSIGNV = 0x8000,
		OR = 0x8001,
		AND = 0x8002,
		XOR = 0x8003,
		ADD = 0x8004,
		SUB_XY = 0x8005,
		SHR = 0x8006,
		SUB_YX = 0x8007,
		SHL = 0x800E,
		SKIP_IFNEQV = 0x9000,
		ASSIGN_ADDRESS = 0xA000,
		JUMP_PLUSV0 = 0xB000,
		RNG_ANDK = 0xC000,
		DRAW = 0xD000,
		SKIP_IF_PRESSED = 0xE09E,
		SKIP_IFN_PRESSED = 0xE0A1,
		GET_DELAY_TIMER = 0xF007,
		WAIT_FOR_KEY = 0xF00A,
		ASSIGN_DELAY_TIMER = 0xF015,
		ASSIGN_SOUND_TIMER = 0xF018,
		ADD_ADDRESS = 0xF01E,
		SET_ADDRESS_TO_FONT = 0xF029,
		STORE_BCD_REP = 0xF033,
		REG_DUMP = 0xF055,
		MEM_DUMP = 0xF065,
	};

	struct interpreter_settings
	{
		int refresh_rate = 60;
		int instructions_per_second = 400;
		// The instruction JUMP_PLUSV0 is supposed to do exactly that, jump to the
		// address given + the value in V0. However, newer interpreters and versions
		// actually jump to the address given plus VX instead of V0. This defines that behavior.
		bool does_jump_plus_vx = true;
		// The shift instruction provides two registers. It's defined to only work on one, but
		// some interpreters assign the value of x to the operation done on y.
		bool shift_xy = false;
		bool increment_addr_reg_during_dump = false;
		std::array<sf::Keyboard::Key, 16> keymap = {
			sf::Keyboard::Key::X,
			sf::Keyboard::Key::Num1,
			sf::Keyboard::Key::Num2,
			sf::Keyboard::Key::Num3,
			sf::Keyboard::Key::Q,
			sf::Keyboard::Key::W,
			sf::Keyboard::Key::E,
			sf::Keyboard::Key::A,
			sf::Keyboard::Key::S,
			sf::Keyboard::Key::D,
			sf::Keyboard::Key::Z,
			sf::Keyboard::Key::C,
			sf::Keyboard::Key::Num4,
			sf::Keyboard::Key::R,
			sf::Keyboard::Key::F,
			sf::Keyboard::Key::V,
		};
		float beep_frequency = 440.F;
	};

	class interpreter
	{
	public:
		interpreter(std::string_view program_directory, const interpreter_settings& settings) : m_memory(program_directory), m_window(settings.beep_frequency)
		{
			m_registers = {};
			m_stack = {};
			m_program_counter = memory::PROGRAM_START;
			m_address_register = 0;
			m_timer = 0;
			m_key_wait = {};
			m_settings = settings;
			m_window.get_keypad().set_keymap(m_settings.keymap);
		}
		memory& get_memory()
		{
			return m_memory;
		}
		window& get_window()
		{
			return m_window;
		}
		void run();
	private:
		memory m_memory;
		window m_window;

		std::array<byte, 16> m_registers;
		std::stack<address> m_stack;
		address m_program_counter;
		address m_address_register;
		int m_timer;
		key_wait m_key_wait;

		interpreter_settings m_settings;

		void do_cycle();
	};
}