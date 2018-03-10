#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

void ShowPrompt()
{
	printf("-");
}

const static string WHITE_SPACE_CHARS{" \t\r\n"};

const size_t MAX_INPUT_BUFFER_SIZE = 4096;

string Trim(const string& s, const string& chars = WHITE_SPACE_CHARS)
{
	auto pos1 = s.find_first_not_of(WHITE_SPACE_CHARS);
	auto pos2 = s.find_last_not_of(WHITE_SPACE_CHARS);
	if (pos1 == string::npos) {
		pos1 = 0;
	}
	if (pos2 == string::npos || pos2 < pos1) {
		pos2 = pos1 - 1;
	}
	return s.substr(pos1, pos2 - pos1 + 1);
}

string GetCommand()
{
	char buf[MAX_INPUT_BUFFER_SIZE] = "";
	fgets(buf, sizeof(buf), stdin);
	return Trim(string{buf});
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

void Process(const string& cmd)
{
	if (cmd == "q") {
		exit(0);
	} else if (cmd == "?") {
		PrintUsage();
	} else if (cmd == "X?") {
		PrintUsageEMS();
	} else {
		fprintf(stderr, "Unsupported command: '%s'\n", cmd.c_str());
	}
}

int main(int argc, char* const* argv)
{
	for (;;) {
		ShowPrompt();
		auto cmd = GetCommand();
		Process(cmd);
	}
	return 0;
}
