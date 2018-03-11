#ifndef COMMAND_PARSER_H__
#define COMMAND_PARSER_H__

#include <string>
#include "Processor.h"
#include "Memory.h"

class CommandParser
{
public:
	CommandParser(Processor& cpu, Memory& memory) : cpu_(cpu), memory_(memory) { }

	std::string GetCommand();
	void Process(const std::string& cmd);
private:
	void ShowPrompt() const;

	std::string prompt_ = "-";

	Processor& cpu_;
	Memory& memory_;
};

#endif
