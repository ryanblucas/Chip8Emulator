#include "interpreter.h"
#include <format>
#include <iostream>
#include <filesystem>

int main(int argc, char** argv)
{
	auto settings = chip8::interpreter_settings{.instructions_per_second = 400};
	// remove this try catch block at the top
	try
	{
		for (const auto& entry : std::filesystem::directory_iterator("./tests"))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".ch8")
			{
				continue;
			}
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