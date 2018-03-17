#ifndef CONSOLE_UI_H__
#define CONSOLE_UI_H__

#include <string>
#include <vector>
#include <utility>
#include "Processor.h"
#include "Registers.h"
#include "Memory.h"

class ConsoleUI
{
public:
	bool Init(const Registers& registers);

	void ShowPrompt();
	std::string ReadLine();

	std::vector<std::pair<size_t, std::string>> SplitCommand(const std::string& cmd);

	void Process(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Processor& processor);
};

#endif
