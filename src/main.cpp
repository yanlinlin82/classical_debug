#include <iostream>
#include "ConsoleUI.h"
#include "Processor.h"
#include "Registers.h"
#include "Memory.h"

int main(int argc, char* const* argv)
{
	Processor processor;
	Registers registers;
	Memory memory;

	ConsoleUI ui;
	if (!ui.Init(registers)) {
		std::cerr << "Error: Failed to initialize console UI!" << std::endl;
		return 1;
	}

	for (;;) {
		ui.ShowPrompt();
		std::string cmd = ui.ReadLine();
		auto words = ui.SplitCommand(cmd);
		if (!words.empty()) {
			ui.Process(words, cmd.size(), processor, registers, memory);
		}
	}
	return 0;
}
