#include "interpreter.h"
#include <format>
#include <iostream>
#include <filesystem>

static std::string parse_args(int argc, char** argv, chip8::interpreter_settings& settings)
{
	std::string result = "./tests";
	for (int i = 1; i < argc; i++)
	{
		std::string_view arg = argv[i];
		if (arg == "--test-path")
		{
			if (i + 1 >= argc)
			{
				throw std::invalid_argument("Expected argument after --test-path\n");
			}
			result = argv[++i];
		}
		else if (arg == "--refresh-rate")
		{
			if (i + 1 >= argc)
			{
				throw std::invalid_argument("Expected argument after --refresh-rate\n");
			}
			settings.refresh_rate = std::stoi(argv[++i]);
		}
		else if (arg == "--ips")
		{
			if (i + 1 >= argc)
			{
				throw std::invalid_argument("Expected argument after --ips\n");
			}
			settings.instructions_per_second = std::stoi(argv[++i]);
		}
		else if (arg == "--beep-frequency")
		{
			if (i + 1 >= argc)
			{
				throw std::invalid_argument("Expected argument after --beep-frequency\n");
			}
			settings.beep_frequency = std::stof(argv[++i]);
		}
		else if (arg == "--keymap") // yeah this parameter sucks
		{
			if (i + 16 >= argc)
			{
				throw std::invalid_argument("Expected 16 arguments after --keymap\n");
			}
			for (int j = 0; j < 16; j++)
			{
				settings.keymap[j] = static_cast<sf::Keyboard::Key>(std::stoi(argv[++i]));
			}
		}
		else if (arg == "--jump-plus-v0")
		{
			settings.does_jump_plus_vx = false;
		}
		else if (arg == "--shift-xy")
		{
			settings.shift_xy = true;
		}
		else if (arg == "--increment-dump-addr")
		{
			settings.increment_addr_reg_during_dump = true;
		}
	}
	return result;
}

int main(int argc, char** argv)
{
	auto settings = chip8::interpreter_settings{};
	std::string test_path = parse_args(argc, argv, settings);
	try
	{
		if (test_path.ends_with(".ch8"))
		{
			std::cout << "Running single program...\n";
			chip8::interpreter program(test_path, settings);
			program.run();
			program.get_window().get_thread().join();
			return 0;
		}

		std::cout << "Running multiple programs. Use ESC to skip to the next program in the directory.\n";
		for (const auto& entry : std::filesystem::directory_iterator(test_path))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".ch8")
			{
				continue;
			}
			std::cout << "Starting program: " << entry.path().filename().string() << "\n";
			chip8::interpreter program(entry.path().string(), settings);
			program.run();
			// some programs like the IBM Logo will run into an infinite loop. The interpreter takes this as a sign
			// to terminate the program while keeping an image up. Therefore, the window must remain open after termination.
			program.get_window().get_thread().join();
			if (program.get_window().get_return_code() != chip8::return_code::ESCAPE_KEY)
			{
				break;
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << std::format("Fatal error: {}\n", e.what());
	}
	return 0;
}