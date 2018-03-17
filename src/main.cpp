#include <iostream>
#include "ConsoleUI.h"
#include "Processor.h"

int main(int argc, char* const* argv)
{
	Processor processor;

	ConsoleUI ui;
	if (!ui.Init(processor.GetRegisters())) {
		std::cerr << "Error: Failed to initialize console UI!" << std::endl;
		return 1;
	}

	for (;;) {
		ui.ShowPrompt();
		std::string cmd = ui.ReadLine();
		auto words = ui.SplitCommand(cmd);
		if (!words.empty()) {
			ui.Process(words, cmd.size(), processor);
		}
	}
	return 0;
}
