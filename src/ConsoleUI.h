#ifndef CONSOLE_UI_H__
#define CONSOLE_UI_H__

#include <string>
#include <vector>
#include <utility>
#include "Processor.h"
#include "Registers.h"
#include "Memory.h"
#include "Command.h"

class ConsoleUI
{
public:
	bool Init(const Registers& registers);

	Command GetCommand();

	std::string ReadLine();

	void Process(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Processor& processor);
	void Process(const Command& cmd, Processor& processor);

private:
	void ShowPrompt();
};

#endif
