#include "interpreter.h"
#include <chrono>
#include <format>
#include <iostream>
#include <cstdlib>

using namespace chip8;

void interpreter::do_cycle()
{
	auto instruction = m_memory.get_instruction(m_program_counter);
	m_program_counter += 2;
	switch (instruction.raw & 0xF000)
	{
	case 0x0000:
	{
		if (instruction.raw == opcode::CLEAR_DISPLAY)
		{
			m_window.get_screen().clear();
		}
		else if (instruction.raw == opcode::RETURN)
		{
			if (m_stack.empty())
			{
				throw std::runtime_error("RET opcode executed when stack pointer is non-positive.");
			}
			m_program_counter = m_stack.top();
			m_stack.pop();
		}
		break;
	}
	case opcode::JUMP:
		m_program_counter = instruction.raw & 0x0FFF;
		break;
	case opcode::CALL:
	{
		m_stack.push(m_program_counter);
		m_program_counter = instruction.raw & 0x0FFF;
		if (m_stack.size() >= 16)
		{
			throw std::runtime_error("Ran out of stack space!");
		}
		break;
	}
	case opcode::SKIP_IFEQK:
		if (m_registers[instruction.nibbles.s2] == instruction.bytes.b2)
		{
			m_program_counter += 2;
		}
		break;
	case opcode::SKIP_IFNEQK:
		if (m_registers[instruction.nibbles.s2] != instruction.bytes.b2)
		{
			m_program_counter += 2;
		}
		break;
	case opcode::SKIP_IFEQV:
		if (m_registers[instruction.nibbles.s2] == m_registers[instruction.nibbles.s3])
		{
			m_program_counter += 2;
		}
		break;
	case opcode::ASSIGNK:
		m_registers[instruction.nibbles.s2] = instruction.bytes.b2;
		break;
	case opcode::ADDK:
		m_registers[instruction.nibbles.s2] += instruction.bytes.b2;
		break;
	case 0x8000:
	{
		// flags here must always be assigned after the operation, if changed at all b/c
		// the register in the opcode could be the flag register
		switch (instruction.raw & 0xF00F)
		{
		case opcode::ASSIGNV:
			m_registers[instruction.nibbles.s2] = m_registers[instruction.nibbles.s3];
			break;
		case opcode::OR:
			m_registers[instruction.nibbles.s2] |= m_registers[instruction.nibbles.s3];
			m_registers[0xF] = 0;
			break;
		case opcode::AND:
			m_registers[instruction.nibbles.s2] &= m_registers[instruction.nibbles.s3];
			m_registers[0xF] = 0;
			break;
		case opcode::XOR:
			m_registers[instruction.nibbles.s2] ^= m_registers[instruction.nibbles.s3];
			m_registers[0xF] = 0;
			break;
		case opcode::ADD:
		{
			int res = m_registers[instruction.nibbles.s2] + m_registers[instruction.nibbles.s3];
			m_registers[instruction.nibbles.s2] = res & 0xFF;
			m_registers[0xF] = 0;
			if ((res & 0xFF) != res)
			{
				m_registers[0xF] = 1;
			}
			break;
		}
		case opcode::SUB_XY:
		{
			byte flag = 0;
			if (m_registers[instruction.nibbles.s2] >= m_registers[instruction.nibbles.s3])
			{
				flag = 1;
			}
			m_registers[instruction.nibbles.s2] -= m_registers[instruction.nibbles.s3];
			m_registers[0xF] = flag;
			break;
		}
		case opcode::SHR:
		{
			int copy = m_registers[instruction.nibbles.s2];
			int operand = m_settings.shift_xy ? m_registers[instruction.nibbles.s3] : m_registers[instruction.nibbles.s2];
			m_registers[instruction.nibbles.s2] = operand >> 1;
			m_registers[0xF] = copy & 1;
			break;
		}
		case opcode::SUB_YX:
		{
			byte flag = 0;
			if (m_registers[instruction.nibbles.s2] <= m_registers[instruction.nibbles.s3])
			{
				flag = 1;
			}
			m_registers[instruction.nibbles.s2] = m_registers[instruction.nibbles.s3] - m_registers[instruction.nibbles.s2];
			m_registers[0xF] = flag;
			break;
		}
		case opcode::SHL:
		{
			int copy = m_registers[instruction.nibbles.s2];
			int operand = m_settings.shift_xy ? m_registers[instruction.nibbles.s3] : m_registers[instruction.nibbles.s2];
			m_registers[instruction.nibbles.s2] = operand << 1;
			m_registers[0xF] = !!(copy & 0x80);
			break;
		}
		default:
			throw std::runtime_error(std::format("Unknown opcode {:04X}", instruction.raw));
			break;
		}
		break;
	}
	case opcode::SKIP_IFNEQV:
		if (m_registers[instruction.nibbles.s2] != m_registers[instruction.nibbles.s3])
		{
			m_program_counter += 2;
		}
		break;
	case opcode::ASSIGN_ADDRESS:
		m_address_register = instruction.raw & 0x0FFF;
		break;
	case opcode::JUMP_PLUSV0:
		m_program_counter = (instruction.raw & 0x0FFF) + (m_settings.does_jump_plus_vx ? m_registers[instruction.nibbles.s2] : m_registers[0x0]);
		break;
	case opcode::RNG_ANDK:
		m_registers[instruction.nibbles.s2] = std::rand() & instruction.bytes.b2;
		break;
	case opcode::DRAW:
	{
		if (instruction.nibbles.s4 == 0)
		{
			// no-op, will fail assert in sprite constructor
			break;
		}
		int x = m_registers[instruction.nibbles.s2];
		int y = m_registers[instruction.nibbles.s3];
		m_registers[0xF] = m_window.get_screen().draw_sprite(x, y, sprite(instruction.nibbles.s4, m_address_register, &m_memory)) ? 1 : 0;
		break;
	}
	case 0xE000:
	case 0xF000:
	{
		auto& arg = m_registers[instruction.nibbles.s2];
		switch (instruction.raw & 0xF0FF)
		{
		case opcode::SKIP_IF_PRESSED:
			if (m_window.get_keypad().is_key_pressed(arg))
			{
				m_program_counter += 2;
			}
			break;
		case opcode::SKIP_IFN_PRESSED:
			if (!m_window.get_keypad().is_key_pressed(arg))
			{
				m_program_counter += 2;
			}
			break;
		case opcode::GET_DELAY_TIMER:
			arg = m_timer;
			break;
		case opcode::WAIT_FOR_KEY:
			m_key_wait.on_opcode(instruction.nibbles.s2, m_window.get_keypad());
			break;
		case opcode::ASSIGN_DELAY_TIMER:
			m_timer = arg;
			break;
		case opcode::ASSIGN_SOUND_TIMER:
			m_window.beep_set(arg);
			break;
		case opcode::ADD_ADDRESS:
			m_address_register += arg;
			break;
		case opcode::SET_ADDRESS_TO_FONT:
			m_address_register = m_memory.PROGRAM_FONT_START + 0x5 * arg;
			break;
		case opcode::STORE_BCD_REP:
		{
			auto value = arg;
			// only 3 decimal digits for an 8-bit value. Trailing zeros are included
			for (int i = 0; i < 3; i++)
			{
				m_memory.set_byte(m_address_register + 2 - i, value % 10);
				value /= 10;
			}
			break;
		}
		case opcode::REG_DUMP:
			for (int i = 0; i <= instruction.nibbles.s2; i++)
			{
				m_memory.set_byte(m_address_register + i, m_registers[i]);
			}
			if (m_settings.increment_addr_reg_during_dump)
			{
				m_address_register = instruction.nibbles.s2 + 1;
			}
			break;
		case opcode::MEM_DUMP:
			for (int i = 0; i <= instruction.nibbles.s2; i++)
			{
				m_registers[i] = m_memory.get_byte(m_address_register + i);
			}
			if (m_settings.increment_addr_reg_during_dump)
			{
				m_address_register = instruction.nibbles.s2 + 1;
			}
			break;
		default:
			throw std::runtime_error(std::format("Unknown opcode {:04X}", instruction.raw));
			break;
		}
		break;
	}
	default:
		throw std::runtime_error(std::format("Unknown opcode {:04X}", instruction.raw));
		break;
	}
}

void interpreter::run()
{
	auto last = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(last.time_since_epoch());
	std::srand(static_cast<unsigned int>(duration.count()));
	while (m_window.get_return_code() == return_code::STILL_RUNNING)
	{
		using namespace std::chrono_literals;
		if (m_key_wait.active)
		{
			m_key_wait.on_wait(m_window.get_keypad(), m_registers);
		}
		else
		{
			// 400 instructions per second is on the lowend, and even that is 2.5ms per instruction.
			// some users might prefer 800 which is 1.25ms, this is just too fast to sleep
			auto cycle_start = std::chrono::steady_clock::now();
			do_cycle();
			while (std::chrono::steady_clock::now() - cycle_start < 1000000000ns / m_settings.instructions_per_second);
		}

		auto curr = std::chrono::steady_clock::now();
		if (curr - last > 1000000000ns / m_settings.refresh_rate)
		{
			m_window.beep_tick();
			m_timer -= m_timer > 0 ? 1 : 0;
			last = curr;
		}
	}
}