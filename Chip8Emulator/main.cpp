#include "interpreter.h"
#include <format>
#include <iostream>

int main()
{
	chip8::interpreter program("Zero Demo [zeroZshadow, 2007].ch8");
	program.run();
	// some programs like the IBM Logo will run into an infinite loop. The interpreter takes this as a sign
	// to terminate the program while keeping an image up. Therefore, the window must remain open after termination.
	program.get_window().get_thread().join();
	return 0;
}