#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "ConsoleUI.h"

int ConsoleUI::KeyboardHit()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

void ConsoleUI::SetNonBlock(ConsoleUI::NonBlock state)
{
	struct termios ttystate;

	//get the terminal state
	tcgetattr(STDIN_FILENO, &ttystate);

	if (state == NB_ENABLE) {
		//turn off canonical mode
		ttystate.c_lflag &= ~(ICANON | ECHO);
		//minimum of number input read.
		ttystate.c_cc[VMIN] = 1;
	} else if (state == NB_DISABLE) {
		//turn on canonical mode
		ttystate.c_lflag |= ICANON | ECHO;
	}
	//set the terminal attributes.
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int ConsoleUI::GetInputChar()
{
	SetNonBlock(NB_ENABLE);
	for (;;) {
		if (KeyboardHit()) {
			int c = fgetc(stdin);
			SetNonBlock(NB_DISABLE);
			return c;
		}
		usleep(0);
	}
}

Command ConsoleUI::GetCommand()
{
	ShowPrompt();
	std::string line = ReadLine();
	Command cmd;
	cmd.Parse(line);
	return cmd;
}

void ConsoleUI::ShowPrompt() const
{
	std::cout << prompt_ << std::flush;
}

std::string ConsoleUI::ReadLine()
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

void ConsoleUI::PrintUsage()
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

void ConsoleUI::ShowError(size_t space, const char* fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	for (size_t i = 0; i < prompt_.size() + space; ++i) {
		fprintf(stdout, " ");
	}
	fprintf(stdout, "^ Error: ");
	vfprintf(stdout, fmt, vl);
	fprintf(stdout, "\n");
	fflush(stdout);
}

bool ParseHex(char c, unsigned char& value)
{
	if (c >= '0' && c <= '9') {
		value = static_cast<unsigned char>(c - '0');
	} else if (c >= 'A' && c <= 'F') {
		value = static_cast<unsigned char>(c - 'A') + 10;
	} else if (c >= 'a' && c <= 'f') {
		value = static_cast<unsigned char>(c - 'a') + 10;
	} else {
		return false;
	}
	return true;
}

template <class T>
bool ParseHex(const std::string& s, T& value)
{
	value = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned char x;
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

bool ConsoleUI::EnsureArgumentCount(const Command& cmd, size_t min, size_t max)
{
	if (cmd.GetWords().size() < min) {
		ShowError(cmd.GetCmdSize(), "Missing argument");
		return false;
	} else if (cmd.GetWords().size() > max) {
		ShowError(cmd.GetWords().back().first, "Unexpected argument");
		return false;
	} else {
		return true;
	}
}

void ConsoleUI::CompareMemory(const Command& cmd, Registers& registers, Memory& memory)
{
	if (!EnsureArgumentCount(cmd, 4, 4)) {
		return;
	}

	unsigned short srcSeg, srcStart, srcEnd;
	unsigned short dstSeg, dstStart;
	size_t errPos;
	std::string errInfo;
	if (!ParseAddress(cmd.GetWords()[1].second, srcSeg, srcStart, errPos, errInfo, registers)) {
		ShowError(cmd.GetWords()[1].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseOffset(cmd.GetWords()[2].second, srcEnd, errPos, errInfo)) {
		ShowError(cmd.GetWords()[2].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseAddress(cmd.GetWords()[3].second, dstSeg, dstStart, errPos, errInfo, registers)) {
		ShowError(cmd.GetWords()[3].first + errPos, errInfo.c_str());
		return;
	}
	memory.Compare(srcSeg, srcStart, srcEnd, dstSeg, dstStart);
}

void ConsoleUI::CopyMemory(const Command& cmd, Registers& registers, Memory& memory)
{
	unsigned short seg, start, end, dstSeg, dstStart;
	size_t errPos;
	std::string errInfo;
	auto words = cmd.GetWords();
	if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseOffset(words[2].second, end, errPos, errInfo)) {
		ShowError(words[2].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseAddress(words[3].second, dstSeg, dstStart, errPos, errInfo, registers)) {
		ShowError(words[3].first + errPos, errInfo.c_str());
		return;
	}
	memory.Copy(seg, start, end, dstSeg, dstStart);
}

void ConsoleUI::EnterData(const Command& cmd, Registers& registers, Memory& memory)
{
	if (cmd.GetWords().size() < 2) {
		ShowError(cmd.GetCmdSize(), "Missing argument");
		return;
	}
	unsigned short seg, start;
	size_t errPos;
	std::string errInfo;
	auto words = cmd.GetWords();
	if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	std::vector<unsigned char> data;
	if (cmd.GetWords().size() > 2) {
		for (size_t i = 2; i < cmd.GetWords().size(); ++i) {
			if (words[i].second[0] == '\'' || words[i].second[0] == '\"') {
				for (size_t j = 0; j < words[i].second.size(); ++j) {
					if (words[i].second[j] != words[i].second[0]) {
						data.push_back(words[i].second[j]);
					}
				}
			} else {
				unsigned char value;
				if (!ParseHex(words[i].second, value)) {
					ShowError(words[i].first, "Invalid hex value '%s'", words[i].second.c_str());
					return;
				}
				data.push_back(value);
			}
		}
	} else {
		std::string word = "";
		bool exitFlag = false;
		for (unsigned short offset = start; !exitFlag; ++offset) {
			if (offset == start || offset % 8 == 0) {
				if (offset != start) {
					printf("\n");
				}
				printf("%04X:%04X  ", seg, offset);
			}
			unsigned char oriValue = memory.GetChar(seg, offset);
			printf("%02X.", oriValue);
			fflush(stdout);
			for (;;) {
				int c = GetInputChar();
				if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
					if (word.size() < 2) {
						printf("%c", c);
						fflush(stdout);
						word += c;
					}
				} else if (c == '\x7F') {
					printf("%c", c);
					fflush(stdout);
					if (word.size() > 0) {
						word.pop_back();
					}
				} else if (c == ' ' || c == '\n') {
					if (word.empty()) {
						printf("  ");
						fflush(stdout);
						data.push_back(oriValue);
					} else {
						unsigned short x;
						ParseHex(word, x);
						if (word.size() == 1) {
							printf(" ");
							fflush(stdout);
						}
						data.push_back(x);
					}
					if (c == '\n') {
						exitFlag = true;
					}
					break;
				}
			}
			printf("   ");
			fflush(stdout);
			word = "";
		}
		printf("\n");
	}
	memory.PutData(seg, start, data);
}

void ConsoleUI::SearchData(const Command& cmd, Registers& registers, Memory& memory)
{
	if (cmd.GetWords().size() < 4) {
		ShowError(cmd.GetCmdSize(), "Missing argument");
		return;
	}
	unsigned short seg, start, end;
	size_t errPos;
	std::string errInfo;
	auto words = cmd.GetWords();
	if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseOffset(words[2].second, end, errPos, errInfo)) {
		ShowError(words[2].first + errPos, errInfo.c_str());
		return;
	}
	std::vector<unsigned char> data;
	for (size_t i = 3; i < cmd.GetWords().size(); ++i) {
		if (words[i].second[0] == '\'' || words[i].second[0] == '\"') {
			for (size_t j = 0; j < words[i].second.size(); ++j) {
				if (words[i].second[j] != words[i].second[0]) {
					data.push_back(words[i].second[j]);
				}
			}
		} else {
			unsigned char x;
			if (!ParseHex(words[i].second, x)) {
				ShowError(words[i].first + errPos, errInfo.c_str());
				return;
			}
			data.push_back(x);
		}
	}
	memory.SearchData(seg, start, end, data);
}

void ConsoleUI::FillData(const Command& cmd, Registers& registers, Memory& memory)
{
	if (cmd.GetWords().size() < 4) {
		ShowError(cmd.GetCmdSize(), "Missing argument");
		return;
	}
	unsigned short seg, start, end;
	size_t errPos;
	std::string errInfo;
	auto words = cmd.GetWords();
	if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseOffset(words[2].second, end, errPos, errInfo)) {
		ShowError(words[2].first + errPos, errInfo.c_str());
		return;
	}
	std::vector<unsigned char> data;
	for (size_t i = 3; i < cmd.GetWords().size(); ++i) {
		unsigned char x;
		if (!ParseHex(words[i].second, x)) {
			ShowError(words[i].first + errPos, errInfo.c_str());
			return;
		}
		data.push_back(x);
	}
	memory.FillData(seg, start, end, data);
}

void ConsoleUI::SetFilename(const std::string& f)
{
	filename_ = f;
}

void ConsoleUI::LoadData(const Command& cmd, Registers& registers, Memory& memory)
{
	if (!EnsureArgumentCount(cmd, 1, 2)) {
		return;
	}
	unsigned short seg, offset;
	if (cmd.GetWords().size() == 1) {
		seg = curSeg_;
		offset = cursor_;
	} else {
		size_t errPos;
		std::string errInfo;
		auto words = cmd.GetWords();
		if (!ParseAddress(words[1].second, seg, offset, errPos, errInfo, registers)) {
			ShowError(words[1].first + errPos, errInfo.c_str());
			return;
		}
	}
	unsigned short size;
	registers.Get("cx", size);
	memory.Load(filename_, seg, offset, size);
}

void ConsoleUI::WriteData(const Command& cmd, Registers& registers, Memory& memory)
{
	if (!EnsureArgumentCount(cmd, 1, 2)) {
		return;
	}
	unsigned short seg, offset;
	if (cmd.GetWords().size() == 1) {
		seg = curSeg_;
		offset = cursor_;
	} else {
		size_t errPos;
		std::string errInfo;
		auto words = cmd.GetWords();
		if (!ParseAddress(words[1].second, seg, offset, errPos, errInfo, registers)) {
			ShowError(words[1].first + errPos, errInfo.c_str());
			return;
		}
	}
	unsigned short size;
	registers.Get("cx", size);
	memory.Write(filename_, seg, offset, size);
}

void ConsoleUI::DumpMemory(const Command& cmd, Registers& registers, Memory& memory)
{
	if (cmd.GetWords().size() > 3) {
		auto words = cmd.GetWords();
		ShowError(words[3].first, "Unexpected argument");
		return;
	}

	unsigned short seg, start, end;
	if (cmd.GetWords().size() == 1) {
		seg = curSeg_;
		start = cursor_;
		end = start + 0x80 - 1;
	} else {
		size_t errPos;
		std::string errInfo;
		auto words = cmd.GetWords();
		if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
			ShowError(words[1].first + errPos, errInfo.c_str());
			return;
		}
		if (cmd.GetWords().size() > 2) {
			if (!ParseOffset(words[2].second, end, errPos, errInfo)) {
				ShowError(words[2].first + errPos, errInfo.c_str());
				return;
			}
		} else {
			end = start + 0x80 - 1;
		}
	}
	memory.Dump(seg, start, end);
	curSeg_ = seg;
	cursor_ = end + 1;
}

void ConsoleUI::SwitchProcessorType(const Command& cmd, Processor& processor)
{
	auto words = cmd.GetWords();
	if (cmd.GetWords().size() == 1 || words[1].second == "?") {
		processor.ShowProcessorType();
	} else if (cmd.GetWords().size() > 2) {
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

void ConsoleUI::HexCalc(const Command& cmd)
{
	if (!EnsureArgumentCount(cmd, 3, 3)) {
		return;
	}
	unsigned short a, b;
	auto words = cmd.GetWords();
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

std::string Trim(const std::string& text)
{
	auto s = text;
	auto start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos) {
		s = "";
	} else {
		s = s.substr(start);
	}
	auto end = s.find_last_not_of(" \t\r\n");
	if (end == std::string::npos) {
		s = "";
	} else {
		s = s.substr(0, end + 1);
	}
	return s;
}

void ConsoleUI::ChangeRegisters(const Command& cmd, Registers& registers)
{
	auto words = cmd.GetWords();
	if (cmd.GetWords().size() > 3) {
		ShowError(words[3].first, "Unexpected argument");
		return;
	}
	std::string regName = words[1].second;
	unsigned short value;
	if (cmd.GetWords().size() == 2) {
		if (!registers.Get(regName, value)) {
			ShowError(words[1].first, "Invalid register name '%s'", regName.c_str());
			return;
		}
		printf("%s %04X  :", regName.c_str(), value);
		char buf[32];
		fgets(buf, sizeof(buf), stdin);
		std::string s = Trim(buf);
		if (s.empty()) {
			return;
		}
		if (!ParseHex(s, value)) {
			ShowError(10, "Invalid hex value '%s'", s.c_str());
			return;
		}
	} else {
		if (!ParseHex(words[2].second, value)) {
			ShowError(words[2].first, "Invalid hex value '%s'", words[2].second.c_str());
			return;
		}
	}
	if (!registers.Set(regName, value)) {
		ShowError(words[1].first, "Invalid register name '%s'", regName.c_str());
		return;
	}
}

void ConsoleUI::Process(const Command& cmd, Processor& processor)
{
	if (cmd.IsEmpty()) {
		return;
	}
	auto words = cmd.GetWords();

	switch (tolower(words[0].second[0])) {
	case 'q':
		exit(0);
	case '?':
		PrintUsage();
		break;
	case 'c':
		CompareMemory(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	case 'd':
		DumpMemory(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	case 'r':
		if (cmd.GetWords().size() == 1) {
			processor.GetRegisters().Dump();
		} else {
			ChangeRegisters(cmd, processor.GetRegisters());
		}
		break;
	case 'm':
		if (cmd.GetWords().size() == 4) {
			CopyMemory(cmd, processor.GetRegisters(), processor.GetMemory());
		} else {
			SwitchProcessorType(cmd, processor);
		}
		break;
	case 'h':
		HexCalc(cmd);
		break;
	case 'e':
		EnterData(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	case 's':
		SearchData(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	case 'f':
		FillData(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	case 'n':
		if (!EnsureArgumentCount(cmd, 2, 2)) {
			return;
		}
		SetFilename(words[1].second);
		break;
	case 'l':
		LoadData(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	case 'w':
		WriteData(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	default:
		ShowError(words[0].first, "Unsupported command '%c'", words[0].second[0]);
	}
}

bool ConsoleUI::Init(const Registers& registers)
{
	curSeg_ = registers.GetDS();
	return true;
}