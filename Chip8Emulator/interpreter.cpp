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
			if (m_stack_pointer <= 0)
			{
				throw std::runtime_error("RET opcode executed when stack pointer is non-positive.");
			}
			m_program_counter = m_stack[--m_stack_pointer];
		}
		break;
	}
	case opcode::JUMP:
	{
		if ((instruction.raw & 0x0FFF) == m_program_counter - 2)
		{
			m_running = false;
		}
		m_program_counter = instruction.raw & 0x0FFF;
		break;
	}
	case opcode::CALL:
	{
		m_stack[m_stack_pointer++] = m_program_counter;
		m_program_counter = instruction.raw & 0x0FFF;
		if (m_stack_pointer >= m_stack.size())
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
		switch (instruction.raw & 0xF00F)
		{
		case opcode::ASSIGNV:
			m_registers[instruction.nibbles.s2] = m_registers[instruction.nibbles.s3];
			break;
		case opcode::OR:
			m_registers[instruction.nibbles.s2] |= m_registers[instruction.nibbles.s3];
			break;
		case opcode::AND:
			m_registers[instruction.nibbles.s2] &= m_registers[instruction.nibbles.s3];
			break;
		case opcode::XOR:
			m_registers[instruction.nibbles.s2] ^= m_registers[instruction.nibbles.s3];
			break;
		case opcode::ADD:
		{
			int res = m_registers[instruction.nibbles.s2] + m_registers[instruction.nibbles.s3];
			m_registers[0xF] = 0;
			if ((res & 0xFF) != res)
			{
				m_registers[0xF] = 1;
			}
			m_registers[instruction.nibbles.s2] = res & 0xFF;
			break;
		}
		case opcode::SUB_XY:
			m_registers[0xF] = 0;
			if (m_registers[instruction.nibbles.s2] > m_registers[instruction.nibbles.s3])
			{
				m_registers[0xF] = 1;
			}
			m_registers[instruction.nibbles.s2] -= m_registers[instruction.nibbles.s3];
			break;
		case opcode::SHR:
			m_registers[0xF] = m_registers[instruction.nibbles.s2] & 1;
			m_registers[instruction.nibbles.s2] >>= 1;
			break;
		case opcode::SUB_YX:
			m_registers[0xF] = 0;
			if (m_registers[instruction.nibbles.s2] < m_registers[instruction.nibbles.s3])
			{
				m_registers[0xF] = 1;
			}
			m_registers[instruction.nibbles.s3] -= m_registers[instruction.nibbles.s2];
			break;
		case opcode::SHL:
			m_registers[0xF] = m_registers[instruction.nibbles.s2] & 0x80;
			m_registers[instruction.nibbles.s2] <<= 1;
			break;
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
		m_program_counter = instruction.raw & 0x0FFF + m_registers[0x0];
		break;
	case opcode::RNG_ANDK:
		m_registers[instruction.nibbles.s2] = std::rand() & instruction.bytes.b2;
		break;
	case opcode::DRAW:
	{
		int x = m_registers[instruction.nibbles.s2];
		int y = m_registers[instruction.nibbles.s3];
		m_window.get_screen().draw_sprite(x, y, sprite(instruction.nibbles.s4, m_address_register, &m_memory));
		break;
	}
	case 0xE000:
	case 0xF000:
	{
		auto arg = instruction.nibbles.s2;
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
			m_registers[arg] = m_timer;
			break;
		case opcode::WAIT_FOR_KEY:
			m_waiting_result = arg + 1;
			break;
		case opcode::ASSIGN_DELAY_TIMER:
			m_timer = m_registers[arg];
			break;
		case opcode::ASSIGN_SOUND_TIMER:
			m_sound_timer = m_registers[arg];
			break;
		case opcode::ADD_ADDRESS:
			m_address_register += m_registers[arg];
			break;
		case opcode::SET_ADDRESS_TO_FONT:
			m_address_register = m_memory.PROGRAM_FONT_START + 0x5 * arg;
			break;
		case opcode::STORE_BCD_REP:
		{
			auto value = m_registers[arg];
			// only 3 decimal digits for an 8-bit value. Trailing zeros are included
			for (int i = 0; i < 3; i++)
			{
				m_memory.set_byte(m_address_register + 2 - i, value % 10);
				value /= 10;
			}
			break;
		}
		case opcode::REG_DUMP:
			for (int i = 0; i <= arg; i++)
			{
				m_memory.set_byte(m_address_register + i, m_registers[i]);
			}
			break;
		case opcode::MEM_DUMP:
			for (int i = 0; i <= arg; i++)
			{
				m_registers[i] = m_memory.get_byte(m_address_register + i);
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
	while (m_window.get_return_code() == return_code::STILL_RUNNING && m_running)
	{
		if (m_waiting_result != 0)
		{
			for (int i = 0; i < 16; i++)
			{
				if (m_window.get_keypad().is_key_pressed(i))
				{
					m_registers[m_waiting_result - 1] = i;
					m_waiting_result = 0;
					break;
				}
			}
		}
		else
		{
			do_cycle();
		}

		auto curr = std::chrono::steady_clock::now();
		using namespace std::chrono_literals;
		if (curr - last > 16666667ns)
		{
			m_sound_timer -= m_sound_timer > 0 ? 1 : 0;
			m_timer -= m_timer > 0 ? 1 : 0;
		}
	}
}