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
		int instructions_per_second;
		// The instruction JUMP_PLUSV0 is supposed to do exactly that, jump to the
		// address given + the value in V0. However, newer interpreters and versions
		// actually jump to the address given plus VX instead of V0. This defines that behavior.
		bool does_jump_plus_vx;
	};

	class interpreter
	{
	public:
		interpreter(std::string_view program_directory, const interpreter_settings& settings) : m_memory(program_directory), m_window()
		{
			m_registers = {};
			m_stack = {};
			m_program_counter = memory::PROGRAM_START;
			m_address_register = 0;
			m_sound_timer = m_timer = 0; 
			m_running = true;
			m_waiting_result = 0;

			using namespace std::chrono_literals;
			m_ns_per_instruction = 1000000000ns / settings.instructions_per_second;
			m_does_jump_plus_vx = settings.does_jump_plus_vx;
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
		int m_sound_timer;
		int m_timer;

		bool m_running;
		int m_waiting_result;

		std::chrono::nanoseconds m_ns_per_instruction;
		bool m_does_jump_plus_vx;

		void do_cycle();
	};
}