#include <iostream>
#include <cstring>
#include <cstdarg>
#include <string>
#include <vector>
#include "Processor.h"
#include "Registers.h"
#include "Memory.h"

static std::string prompt = "-";
unsigned short curSeg;
unsigned short cursor = 0x100;

void ShowPrompt()
{
	std::cout << prompt << std::flush;
}

std::string ReadLine()
{
	static const size_t BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE];
	fgets(buffer, sizeof(buffer), stdin);
	size_t size = strlen(buffer);
	if (size > 0 && buffer[size - 1] == '\n') {
		buffer[size - 1] = '\0';
	}
	return buffer;
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

void ShowError(size_t space, const char* fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	for (size_t i = 0; i < prompt.size() + space; ++i) {
		fprintf(stdout, " ");
	}
	fprintf(stdout, "^ Error: ");
	vfprintf(stdout, fmt, vl);
	fprintf(stdout, "\n");
	fflush(stdout);
}

bool ParseHex(char c, unsigned short& value)
{
	if (c >= '0' && c <= '9') {
		value = static_cast<unsigned short>(c - '0');
	} else if (c >= 'A' && c <= 'F') {
		value = static_cast<unsigned short>(c - 'A') + 10;
	} else if (c >= 'a' && c <= 'f') {
		value = static_cast<unsigned short>(c - 'a') + 10;
	} else {
		return false;
	}
	return true;
}

bool ParseHex(const std::string& s, unsigned short& value)
{
	value = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned short x;
		if (!ParseHex(s[i], x)) {
			return false;
		}
		value = value * 16 + x;
	}
	return true;
}

bool ParseOffset(const std::string& s, unsigned short& offset, size_t& errPos, std::string& errInfo)
{
	if (!ParseHex(s, offset)) {
		errPos = 0;
		errInfo = "Invalid offset value '" + s + "'";
		return false;
	}
	return true;
}

bool ParseAddress(const std::string& s, unsigned short& seg, unsigned short& offset, size_t& errPos, std::string& errInfo, Registers& registers)
{
	auto text = s;
	size_t skip = 0;
	auto pos = text.find_first_of(':');
	if (pos == std::string::npos) {
		seg = registers.GetDS();
	} else {
		skip = pos + 1;
		auto segReg = text.substr(0, pos);
		text = text.substr(pos + 1);
		if (!registers.GetSeg(segReg, seg) && !ParseHex(segReg, seg)) {
			errPos = 0;
			errInfo = "Invalid segment register/value '" + segReg + "'";
			return false;
		}
	}
	if (!ParseOffset(text, offset, errPos, errInfo)) {
		errPos += skip;
		return false;
	}
	return true;
}

bool EnsureArgumentCount(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, size_t min, size_t max)
{
	if (words.size() < min) {
		ShowError(cmdSize, "Missing argument");
		return false;
	} else if (words.size() > max) {
		ShowError(words[4].first, "Unexpected argument");
		return false;
	} else {
		return true;
	}
}

void CompareMemory(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Registers& registers, Memory& memory)
{
	if (!EnsureArgumentCount(words, cmdSize, 4, 4)) {
		return;
	}

	unsigned short srcSeg, srcStart, srcEnd;
	unsigned short dstSeg, dstStart;
	size_t errPos;
	std::string errInfo;
	if (!ParseAddress(words[1].second, srcSeg, srcStart, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseOffset(words[2].second, srcEnd, errPos, errInfo)) {
		ShowError(words[2].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseAddress(words[3].second, dstSeg, dstStart, errPos, errInfo, registers)) {
		ShowError(words[3].first + errPos, errInfo.c_str());
		return;
	}
	memory.Compare(srcSeg, srcStart, srcEnd, dstSeg, dstStart);
}

void DumpMemory(const std::vector<std::pair<size_t, std::string>>& words, Registers& registers, Memory& memory)
{
	if (words.size() > 3) {
		ShowError(words[3].first, "Unexpected argument");
		return;
	}

	unsigned short seg, start, end;
	if (words.size() == 1) {
		seg = curSeg;
		start = cursor;
		end = start + 0x80 - 1;
	} else {
		size_t errPos;
		std::string errInfo;
		if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
			ShowError(words[1].first + errPos, errInfo.c_str());
			return;
		}
		if (words.size() > 2) {
			if (!ParseOffset(words[2].second, end, errPos, errInfo)) {
				ShowError(words[2].first + errPos, errInfo.c_str());
				return;
			}
		} else {
			end = start + 0x80 - 1;
		}
	}
	memory.Dump(seg, start, end);
	curSeg = seg;
	cursor = end + 1;
}

void SwitchProcessorType(const std::vector<std::pair<size_t, std::string>>& words, Processor& processor)
{
	if (words.size() == 1 || words[1].second == "?") {
		processor.ShowProcessorType();
	} else if (words.size() > 2) {
		ShowError(words[2].first, "Unexpected argument");
	} else if (words[1].second == "0") {
		processor.SetProcessorType(ProcessorType::PT_8086);
	} else if (words[1].second == "1") {
		processor.SetProcessorType(ProcessorType::PT_186);
	} else if (words[1].second == "2") {
		processor.SetProcessorType(ProcessorType::PT_286);
	} else if (words[1].second == "3") {
		processor.SetProcessorType(ProcessorType::PT_386);
	} else if (words[1].second == "4") {
		processor.SetProcessorType(ProcessorType::PT_486);
	} else if (words[1].second == "5") {
		processor.SetProcessorType(ProcessorType::PT_586);
	} else if (words[1].second == "6") {
		processor.SetProcessorType(ProcessorType::PT_686);
	} else if (words[1].second == "nc") {
		processor.SetCoProcessorType(CoProcessorType::CPT_NONE);
	} else if (words[1].second == "c2") {
		processor.SetCoProcessorType(CoProcessorType::CPT_287);
	} else if (words[1].second == "c") {
		processor.SetCoProcessorType(CoProcessorType::CPT_387);
	} else {
		ShowError(words[1].first, "Unexpected command '%s'", words[1].second.c_str());
	}
}

void HexCalc(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize)
{
	if (!EnsureArgumentCount(words, cmdSize, 3, 3)) {
		return;
	}
	unsigned short a, b;
	if (!ParseHex(words[1].second, a)) {
		ShowError(words[1].first, "Unexpected hex value '%s'", words[1].second.c_str());
		return;
	}
	if (!ParseHex(words[2].second, b)) {
		ShowError(words[2].first, "Unexpected hex value '%s'", words[2].second.c_str());
		return;
	}
	printf("%04X  %04X\n",
			static_cast<unsigned short>(a + b),
			static_cast<unsigned short>(a - b));
}

void Process(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Processor& processor, Registers& registers, Memory& memory)
{
	switch (tolower(words[0].second[0])) {
	case 'q':
		exit(0);
	case '?':
		PrintUsage();
		break;
	case 'x':
		if (words[2].second == "?") {
			PrintUsageEMS();
		}
		break;
	case 'c':
		CompareMemory(words, cmdSize, registers, memory);
		break;
	case 'd':
		DumpMemory(words, registers, memory);
		break;
	case 'r':
		processor.ShowRegisters();
		break;
	case 'm':
		SwitchProcessorType(words, processor);
		break;
	case 'h':
		HexCalc(words, cmdSize);
		break;
	default:
		ShowError(words[0].first, "Unsupported command '%c'", words[0].second[0]);
	}
}

std::vector<std::pair<size_t, std::string>> SplitCommand(const std::string& cmd)
{
	const static std::string WHITE_CHARS = " \t\r\n";
	std::vector<std::pair<size_t, std::string>> words;
	std::string::size_type pos = 0;
	while (pos != std::string::npos) {
		std::string::size_type start = cmd.find_first_not_of(WHITE_CHARS, pos);
		if (start == std::string::npos) {
			break;
		}
		std::string::size_type end = cmd.find_first_of(WHITE_CHARS, start);
		std::string word = cmd.substr(start, end - start);
		if (words.empty() && word.size() > 1) {
			words.push_back(std::make_pair(start, word.substr(0, 1)));
			words.push_back(std::make_pair(start + 1, word.substr(1)));
		} else {
			words.push_back(std::make_pair(start, word));
		}
		pos = end;
	}
#if 0
	std::cerr << "[DEBUG] Split: '" << cmd << "' =>\n";
	for (auto x : words) {
		std::cerr << "[DEBUG]    (pos = " << x.first << "): '" << x.second << "'\n";
	}
	std::cerr << std::flush;
#endif
	return words;
}

int main(int argc, char* const* argv)
{
	Processor processor;
	Registers registers;
	Memory memory;
	curSeg = registers.GetDS();
	for (;;) {
		ShowPrompt();
		std::string cmd = ReadLine();
		auto words = SplitCommand(cmd);
		if (!words.empty()) {
			Process(words, cmd.size(), processor, registers, memory);
		}
	}
	return 0;
}
