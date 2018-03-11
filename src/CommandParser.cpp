#include <cstdio>
#include <iostream>
#include "CommandParser.h"
#include "Utilities.h"

std::string CommandParser::GetCommand()
{
	ShowPrompt();

	const size_t MAX_INPUT_BUFFER_SIZE = 4096;
	char buf[MAX_INPUT_BUFFER_SIZE] = "";
	fgets(buf, sizeof(buf), stdin);
	return Utilities::Trim(std::string{buf});
}

void CommandParser::ShowPrompt() const
{
	std::cout << prompt_ << std::flush;
}

void PrintUsage()
{
	printf("Classical Debug (v0.01)\n");
	printf("Altering memory:\n");
	printf("compare      C range address            hex add/sub  H value1 value2\n");
	printf("dump         D [range]                  move         M range address\n");
	printf("enter        E address [list]           search       S range list\n");
	printf("fill         F range list               expanded mem XA/XD/XM/XS (X? for help)\n");
	printf("\n");
	printf("Assemble/Disassemble:\n");
	printf("assemble     A [address]                unassemble   U [range]\n");
	printf("80x86 mode   M x (0..6, ? for query)    set FPU mode MC 387, MNC none, MC2 287\n");
	printf("\n");
	printf("Program execution:\n");
	printf("go           G [=address] [breakpts]    quit         Q\n");
	printf("proceed      P [=address] [count]       trace        T [=address] [count]\n");
	printf("register     R register [value]         all regs     R\n");
	printf("input        I port                     output       O port type\n");
	printf("\n");
	printf("Disk access:\n");
	printf("set name     N [[drive:][path]progname [arglist]]\n");
	printf("load program L [address]\n");
	printf("load         L address drive sector number\n");
	printf("write prog.  W [address]\n");
	printf("write        W address drive sector number\n");
}

void PrintUsageEMS()
{
	printf("Expanded memory (EMS) commands:\n");
	printf("  Allocate      XA count\n");
	printf("  Deallocate    XD handle\n");
	printf("  Map memory    XM logical-page physical-page handle\n");
	printf("  Show status   XS\n");
}

void CommandParser::Process(const std::string& cmd)
{
	if (cmd == "q") {
		exit(0);
	} else if (cmd == "?") {
		PrintUsage();
	} else if (cmd == "X?") {
		PrintUsageEMS();
	} else if (cmd == "d") {
		memory_.Dump(cpu_.GetDataSegment(), 0, 0);
	} else if (cmd == "r") {
		cpu_.ShowRegisters();
	} else if (cmd[0] == 'm') {
		if (cmd[1] >= '0' && cmd[1] <= '6') {
			cpu_.SetProcessorType(static_cast<ProcessorType>(cmd[1] - '0'));
		} else if (cmd[1] == '\0' || cmd[1] == '?') {
			cpu_.ShowProcessorType();
		} else if (cmd == "mnc") {
			cpu_.SetCoProcessorType(CoProcessorType::CPT_NONE);
		} else if (cmd == "mc2") {
			cpu_.SetCoProcessorType(CoProcessorType::CPT_287);
		} else if (cmd == "mc") {
			cpu_.SetCoProcessorType(CoProcessorType::CPT_387);
		} else {
			printf("  ^ Error\n");
		}
	} else {
		fprintf(stderr, "Unsupported command: '%s'\n", cmd.c_str());
	}
}