#include <cstdio>
#include <cstdlib>
#include <string>

using namespace std;

void ShowPrompt()
{
	printf("-");
}

const static string WHITE_SPACE_CHARS{" \t\r\n"};

const size_t MAX_INPUT_BUFFER_SIZE = 4096;

unsigned short ax = 0;
unsigned short bx = 0;
unsigned short cx = 0;
unsigned short dx = 0;
unsigned short sp = 0xFFFE;
unsigned short bp = 0;
unsigned short si = 0;
unsigned short di = 0;
unsigned short ds = 0x07BE;
unsigned short es = 0x07BE;
unsigned short ss = 0x07BE;
unsigned short cs = 0x07BE;
unsigned short ip = 0x0100;
unsigned short flag = 0;

unsigned char memory[65536];
unsigned short cursor = 0x100;

enum ProcessorType
{
	PROCESSOR_8086 = 0,
	PROCESSOR_186,
	PROCESSOR_286,
	PROCESSOR_386,
	PROCESSOR_486,
	PROCESSOR_586,
	PROCESSOR_686
};

enum CoProcessorType
{
	COPROCESSOR_NONE = 0,
	COPROCESSOR_287,
	COPROCESSOR_387
};

ProcessorType processor = PROCESSOR_686;
CoProcessorType coprocessor = COPROCESSOR_387;

const static string PROCESSOR_NAMES[] = {
	"8086/88",
	"186",
	"286",
	"386",
	"486",
	"586",
	"686"
};

const static string CO_PROCESSOR_NAMES[] = {
	"without coprocessor",
	"with 287",
	"with coprocessor"
};

void SetProcessorType(ProcessorType type)
{
	processor = type;
}

void SetCoProcessorType(CoProcessorType type)
{
	coprocessor = type;
}

void ShowProcessorType()
{
	printf("%s %s\n", PROCESSOR_NAMES[processor].c_str(),
			CO_PROCESSOR_NAMES[coprocessor].c_str());
}

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

void DumpMemory()
{
	for (int i = 0; i < 8; ++i) {
		printf("%04X:%04X ", ds, cursor);
		for (int j = 0; j < 16; ++j) {
			printf(" %02X", memory[cursor + j]);
		}
		printf("   ");
		for (int j = 0; j < 16; ++j) {
			unsigned char c = memory[cursor + j];
			printf("%c", (c >= 0x20 && c < 0x7F) ? c : '.');
		}
		printf("\n");
		cursor += 16;
	}
}

void ShowRegisters()
{
	printf("AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n",
			ax, bx, cx, dx, sp, bp, si, di);
	printf("DS=%04X  ES=%04X  SS=%04X  CS=%04X  IP=%04X   NV UP DI PL NZ NA PO NC\n",
			ds, es, ss, cs, ip);
	printf("%04X:%04X F60000        TEST    BYTE PTR [BX+SI],00                  DS:0000=CD\n", cs, ip);
}

void Process(const string& cmd)
{
	if (cmd == "q") {
		exit(0);
	} else if (cmd == "?") {
		PrintUsage();
	} else if (cmd == "X?") {
		PrintUsageEMS();
	} else if (cmd == "d") {
		DumpMemory();
	} else if (cmd == "r") {
		ShowRegisters();
	} else if (cmd[0] == 'm') {
		if (cmd[1] >= '0' && cmd[1] <= '6') {
			SetProcessorType(static_cast<ProcessorType>(cmd[1] - '0'));
		} else if (cmd[1] == '\0' || cmd[1] == '?') {
			ShowProcessorType();
		} else if (cmd == "mnc") {
			SetCoProcessorType(COPROCESSOR_NONE);
		} else if (cmd == "mc2") {
			SetCoProcessorType(COPROCESSOR_287);
		} else if (cmd == "mc") {
			SetCoProcessorType(COPROCESSOR_387);
		} else {
			printf("  ^ Error\n");
		}
	} else {
		fprintf(stderr, "Unsupported command: '%s'\n", cmd.c_str());
	}
}

void InitMemroy()
{
	srand(123);
	for (size_t i = 0; i < sizeof(memory); ++i) {
		memory[i] = static_cast<unsigned char>(rand());
	}
}

int main(int argc, char* const* argv)
{
	InitMemroy();
	for (;;) {
		ShowPrompt();
		auto cmd = GetCommand();
		Process(cmd);
	}
	return 0;
}
