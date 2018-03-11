#include "Processor.h"
#include "CommandParser.h"

int main(int argc, char* const* argv)
{
	Processor cpu;
	Memory memory;

	CommandParser parser(cpu, memory);
	for (;;) {
		auto cmd = parser.GetCommand();
		parser.Process(cmd);
	}
	return 0;
}
