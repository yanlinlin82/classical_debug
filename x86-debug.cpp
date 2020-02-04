#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <locale>
#include <utility>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
using namespace std;

enum class ProcessorType { PT_8086 = 0, PT_186, PT_286, PT_386, PT_486, PT_586, PT_686 };

enum class CoProcessorType { CPT_NONE = 0, CPT_287, CPT_387 };

class Registers
{
public:
	enum {
		FLAGS = 0,

		AX, BX, CX, DX, SP, BP, SI, DI,
		DS, ES, SS, CS, IP,

		MAX_REG_INDEX,
		MIN_REG_INDEX = 1
	};

	void Dump();

	unsigned short GetDS() const { return regs_[DS]; }
	bool GetSeg(const string& name, unsigned short& value) const;
	bool Get(const string& name, unsigned short& value) const;
	bool Set(const string& name, unsigned short value);
private:
	unsigned regs_[MAX_REG_INDEX] = {
		0, // FLAGS
		0, 0, 0, 0, 0xFFFE, 0, 0, 0, // AX, BX, CX, DX, SP, BP, SI, DI
		0x07BE, 0x07BE, 0x07BE, 0x07BE, 0x0100  // DS, ES, SS, CS, IP
	};
};

class Memory
{
public:
	Memory();

	void Dump(unsigned short seg, unsigned short start, unsigned short end);

	void Compare(unsigned short srcSeg, unsigned short srcStart, unsigned short srcEnd,
			unsigned short dstSeg, unsigned short dstStart);
	void Copy(unsigned short srcSeg, unsigned short srcStart, unsigned short srcEnd,
			unsigned short dstSeg, unsigned short dstStart);
	void PutData(unsigned short srcSeg, unsigned short srcStart, vector<unsigned char> data);

	unsigned char GetChar(unsigned short seg, unsigned short offset) const;
	void SearchData(unsigned short seg, unsigned short start, unsigned short end,
			const vector<unsigned char>& data);
	void FillData(unsigned short seg, unsigned short start, unsigned short end,
			const vector<unsigned char>& data);

	bool Load(const string& filename, unsigned short seg,
			unsigned short offset, unsigned short& size);
	bool Write(const string& filename, unsigned short seg,
			unsigned short offset, unsigned short size);
	bool Unassemble(unsigned short seg, unsigned short offset) const;
private:
	bool ParseOneInstrument(const unsigned char* p, size_t& count, string& op, string& operands) const;
	static string Hex(unsigned char x);
private:
	vector<unsigned char> data_;
};

class Processor
{
public:
	void SetProcessorType(ProcessorType type);
	void SetCoProcessorType(CoProcessorType type);
	void ShowProcessorType();

	Registers& GetRegisters() { return registers_; }
	Memory& GetMemory() { return memory_; }

	const Registers& GetRegisters() const { return registers_; }
	const Memory& GetMemory() const { return memory_; }
private:
	ProcessorType processor{ProcessorType::PT_686};
	CoProcessorType coprocessor{CoProcessorType::CPT_387};
private:
	Registers registers_;
	Memory memory_;
};

class Command
{
public:
	void Parse(const string& cmd);
	void Dump() const;

	bool IsEmpty() const { return words_.empty(); }

	const vector<pair<size_t, string>> GetWords() const { return words_; }
	size_t GetCmdSize() const { return cmd_.size(); }
private:
	string cmd_;
	vector<pair<size_t, string>> words_;
};

class Console
{
public:
	int GetInputChar();
private:
	enum NonBlock { NB_ENABLE, NB_DISABLE };
	void SetNonBlock(NonBlock state);
	int KeyboardHit();
};

class ConsoleUI
{
public:
	bool Init(Registers& registers, Memory& memory, const vector<string>& args);

	Command GetCommand();

	void Process(const Command& cmd, Processor& processor);
private:
	void ShowError(size_t space, const char* fmt, ...);
	bool EnsureArgumentCount(const Command& cmd, size_t min, size_t max);

	void CompareMemory(const Command& cmd, Registers& registers, Memory& memory);
	void CopyMemory(const Command& cmd, Registers& registers, Memory& memory);
	void EnterData(const Command& cmd, Registers& registers, Memory& memory);
	void SearchData(const Command& cmd, Registers& registers, Memory& memory);
	void FillData(const Command& cmd, Registers& registers, Memory& memory);

	void SetFilename(const string& f);
	void LoadData(const Command& cmd, Registers& registers, Memory& memory);
	void WriteData(const Command& cmd, Registers& registers, Memory& memory);
	void DumpMemory(const Command& cmd, Registers& registers, Memory& memory);
	void SwitchProcessorType(const Command& cmd, Processor& processor);
	void HexCalc(const Command& cmd);
	void ChangeRegisters(const Command& cmd, Registers& registers);
	void Unassemble(const Command& cmd, Registers& registers, Memory& memory);

	static void PrintUsage();
private:
	void ShowPrompt() const;
	string ReadLine();

	string prompt_ = "-";
	unsigned short curSeg_;
	unsigned short cursor_ = 0x100;

	string filename_ = "";
	vector<string> args_;
};

void Command::Parse(const string& cmd)
{
	words_.clear();

	size_t pos = 0;
	string word = "";
	char quote = '\0';
	for (size_t i = 0; i < cmd.size(); ++i) {
		char c = cmd[i];
		if (quote == '\0' && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
			if (!word.empty()) {
				if (words_.empty() && word.size() > 1) {
					words_.push_back(make_pair(pos, word.substr(0, 1)));
					words_.push_back(make_pair(pos + 1, word.substr(1)));
				} else {
					words_.push_back(make_pair(pos, word));
				}
			}
			word = "";
		} else {
			if (word.empty()) {
				pos = i;
			}
			if (c == '\'' || c == '\"') {
				word += c;
				if (quote == '\0') {
					quote = c;
				} else {
					quote = '\0';
				}
			} else if (c == '\\') {
				if (i + 1 >= cmd.size()) {
					break;
				}
				switch (cmd[++i]) {
				case 'r': word += '\r'; break;
				case 'n': word += '\n'; break;
				case '\\': word += '\\'; break;
				case '\'': word += '\''; break;
				case '\"': word += '\"'; break;
				default: word += cmd[i + 1]; break;
				}
			} else {
				word += c;
			}
		}
	}
	if (!word.empty()) {
		if (words_.empty() && word.size() > 1) {
			words_.push_back(make_pair(pos, word.substr(0, 1)));
			words_.push_back(make_pair(pos + 1, word.substr(1)));
		} else {
			words_.push_back(make_pair(pos, word));
		}
	}
	cmd_ = cmd;

	//Dump();
}

void Command::Dump() const
{
	cerr << "[DEBUG] Split: '" << cmd_ << "' =>\n";
	for (auto x : words_) {
		cerr << "[DEBUG]    (pos = " << x.first << "): '" << x.second << "'\n";
	}
	cerr << flush;
}

int Console::KeyboardHit()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

void Console::SetNonBlock(Console::NonBlock state)
{
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate);
	if (state == NB_ENABLE) {
		ttystate.c_lflag &= ~(ICANON | ECHO); // turn off 'canonical' and 'echo' mode
		ttystate.c_cc[VMIN] = 1; // minimum of number input read.
	} else if (state == NB_DISABLE) {
		ttystate.c_lflag |= ICANON | ECHO; // turn on 'canonical' and 'echo' mode
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int Console::GetInputChar()
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

bool ConsoleUI::Init(Registers& registers, Memory& memory, const vector<string>& args)
{
	curSeg_ = registers.GetDS();
	if (!args.empty()) {
		filename_ = args[0];
		unsigned short size;
		if (!memory.Load(filename_, curSeg_, cursor_, size)) {
			return false;
		}
		registers.Set("cx", size);
		if (args.size() > 1) {
			args_ = vector<string>(args.begin() + 1, args.end());
		}
	}
	return true;
}

Command ConsoleUI::GetCommand()
{
	ShowPrompt();
	string line = ReadLine();
	Command cmd;
	cmd.Parse(line);
	return cmd;
}

void ConsoleUI::ShowPrompt() const
{
	cout << prompt_ << flush;
}

string ConsoleUI::ReadLine()
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
bool ParseHex(const string& s, T& value)
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

bool ParseOffset(const string& s, unsigned short& offset, size_t& errPos, string& errInfo)
{
	if (!ParseHex(s, offset)) {
		errPos = 0;
		errInfo = "Invalid offset value '" + s + "'";
		return false;
	}
	return true;
}

bool ParseAddress(const string& s, unsigned short& seg, unsigned short& offset, size_t& errPos, string& errInfo, Registers& registers)
{
	auto text = s;
	size_t skip = 0;
	auto pos = text.find_first_of(':');
	if (pos == string::npos) {
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
	string errInfo;
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
	string errInfo;
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
	string errInfo;
	auto words = cmd.GetWords();
	if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	vector<unsigned char> data;
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
		string word = "";
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
				Console console;
				int c = console.GetInputChar();
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
	string errInfo;
	auto words = cmd.GetWords();
	if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseOffset(words[2].second, end, errPos, errInfo)) {
		ShowError(words[2].first + errPos, errInfo.c_str());
		return;
	}
	vector<unsigned char> data;
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
	string errInfo;
	auto words = cmd.GetWords();
	if (!ParseAddress(words[1].second, seg, start, errPos, errInfo, registers)) {
		ShowError(words[1].first + errPos, errInfo.c_str());
		return;
	}
	if (!ParseOffset(words[2].second, end, errPos, errInfo)) {
		ShowError(words[2].first + errPos, errInfo.c_str());
		return;
	}
	vector<unsigned char> data;
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

void ConsoleUI::SetFilename(const string& f)
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
		string errInfo;
		auto words = cmd.GetWords();
		if (!ParseAddress(words[1].second, seg, offset, errPos, errInfo, registers)) {
			ShowError(words[1].first + errPos, errInfo.c_str());
			return;
		}
	}
	unsigned short size;
	memory.Load(filename_, seg, offset, size);
	registers.Set("cx", size);
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
		string errInfo;
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

void ConsoleUI::Unassemble(const Command& cmd, Registers& registers, Memory& memory)
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
		string errInfo;
		auto words = cmd.GetWords();
		if (!ParseAddress(words[1].second, seg, offset, errPos, errInfo, registers)) {
			ShowError(words[1].first + errPos, errInfo.c_str());
			return;
		}
	}
	memory.Unassemble(seg, offset);
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
		string errInfo;
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

string Trim(const string& text)
{
	auto s = text;
	auto start = s.find_first_not_of(" \t\r\n");
	if (start == string::npos) {
		s = "";
	} else {
		s = s.substr(start);
	}
	auto end = s.find_last_not_of(" \t\r\n");
	if (end == string::npos) {
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
	string regName = words[1].second;
	unsigned short value;
	if (cmd.GetWords().size() == 2) {
		if (!registers.Get(regName, value)) {
			ShowError(words[1].first, "Invalid register name '%s'", regName.c_str());
			return;
		}
		printf("%s %04X  :", regName.c_str(), value);
		char buf[32];
		fgets(buf, sizeof(buf), stdin);
		string s = Trim(buf);
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
	case 'u':
		Unassemble(cmd, processor.GetRegisters(), processor.GetMemory());
		break;
	default:
		ShowError(words[0].first, "Unsupported command '%c'", words[0].second[0]);
	}
}

Memory::Memory()
{
	data_.resize(1024 * 1024); // 1MB memory for 'real mode'

	srand(123);
	for (size_t i = 0; i < data_.size(); ++i) {
		data_[i] = static_cast<unsigned char>(rand());
	}
}

void Memory::Dump(unsigned short seg, unsigned short start, unsigned short end)
{
	unsigned short cursor = start & 0xFFF0;
	size_t realStart = seg * 16 + start;
	size_t realEnd = seg * 16 + end;
	if (realEnd < realStart) {
		realEnd = realStart;
	}
	bool exitFlag = false;
	while (!exitFlag) {
		printf("%04X:%04X ", seg, cursor);
		for (int j = 0; j < 16; ++j) {
			printf("%c", (j == 8 ? '-' : ' '));
			size_t realOffset = seg * 16 + cursor + j;
			if (realOffset < realStart || realOffset > realEnd) {
				printf("  ");
			} else {
				printf("%02X", data_[realOffset]);
			}
		}
		printf("   ");
		for (int j = 0; j < 16; ++j) {
			size_t realOffset = seg * 16 + cursor + j;
			if (realOffset < realStart || realOffset > realEnd) {
				printf(" ");
			} else {
				unsigned char c = data_[realOffset];
				printf("%c", (c >= 0x20 && c < 0x7F) ? c : '.');
			}
			if (realOffset >= realEnd) {
				exitFlag = true;
			}
		}
		printf("\n");
		cursor += 16;
	}
}

void Memory::Compare(unsigned short srcSeg, unsigned short srcStart, unsigned short srcEnd,
		unsigned short dstSeg, unsigned short dstStart)
{
	unsigned short srcOffset = srcStart;
	unsigned short dstOffset = dstStart;
	for (;;) {
		size_t srcRealOffset = srcSeg * 16 + srcOffset;
		size_t dstRealOffset = dstSeg * 16 + dstOffset;
		if (data_[srcRealOffset] != data_[dstRealOffset]) {
			printf("%04X:%04X  %02X %02X  %04X:%04X\n",
					srcSeg, srcOffset, data_[srcRealOffset],
					data_[dstRealOffset], dstSeg, dstOffset);
		}
		if (srcOffset == srcEnd) {
			break;
		}
		++srcOffset;
		++dstOffset;
	}
}

void Memory::Copy(unsigned short srcSeg, unsigned short srcStart, unsigned short srcEnd,
		unsigned short dstSeg, unsigned short dstStart)
{
	unsigned short srcOffset = srcStart;
	unsigned short dstOffset = dstStart;
	for (;;) {
		size_t srcRealOffset = srcSeg * 16 + srcOffset;
		size_t dstRealOffset = dstSeg * 16 + dstOffset;
		data_[dstRealOffset] = data_[srcRealOffset];
		if (srcOffset == srcEnd) {
			break;
		}
		++srcOffset;
		++dstOffset;
	}
}

void Memory::PutData(unsigned short seg, unsigned short start, vector<unsigned char> data)
{
	size_t offset = seg * 16 + start;
	for (size_t i = 0; i < data.size(); ++i) {
		data_[offset + i] = data[i];
	}
}

unsigned char Memory::GetChar(unsigned short seg, unsigned short offset) const
{
	size_t realOffset = seg * 16 + offset;
	return data_[realOffset];
}

void Memory::SearchData(unsigned short seg, unsigned short start, unsigned short end,
		const vector<unsigned char>& data)
{
	for (unsigned short offset = start; offset + data.size() != end; ++offset) {
		size_t realOffset = seg * 16 + offset;
		bool match = true;
		for (size_t i = 0; i < data.size(); ++i) {
			if (data_[realOffset + i] != data[i]) {
				match = false;
				break;
			}
		}
		if (match) {
			printf("%04X:%04X\n", seg, offset);
		}
	}
}

void Memory::FillData(unsigned short seg, unsigned short start, unsigned short end,
		const vector<unsigned char>& data)
{
	size_t i = 0;
	for (unsigned short offset = start; ; ++offset, ++i) {
		size_t realOffset = seg * 16 + offset;
		data_[realOffset] = data[i % data.size()];
		if (offset == end) {
			break;
		}
	}
}

bool Memory::Load(const string& filename, unsigned short seg,
		unsigned short offset, unsigned short& size)
{
	unsigned short realOffset = seg * 16 + offset;
	size = 0;
	FILE* fp = fopen(filename.c_str(), "r");
	if (!fp) {
		return false;
	}
	while (!feof(fp)) {
		char buf[1024];
		size_t n = fread(buf, 1, sizeof(buf), fp);
		if (n > 0) {
			memcpy(&data_[realOffset], buf, n);
			realOffset += n;
			size += n;
		}
	}
	fclose(fp);
	return true;
}

bool Memory::Write(const string& filename, unsigned short seg,
		unsigned short offset, unsigned short size)
{
	unsigned short realOffset = seg * 16 + offset;
	FILE* fp = fopen(filename.c_str(), "w");
	if (!fp) {
		return false;
	}
	unsigned short BLOCK_SIZE = 1024;
	for (size_t i = 0; i < size; ) {
		size_t count = size - i;
		if (count > BLOCK_SIZE) {
			count = BLOCK_SIZE;
		}
		size_t n = fwrite(&data_[realOffset], 1, count, fp);
		realOffset += n;
		i += n;
	}
	fclose(fp);
	return true;
}

string Memory::Hex(unsigned char x)
{
	char buf[16] = "";
	snprintf(buf, sizeof(buf), "%02X", x);
	return buf;
}

bool Memory::ParseOneInstrument(const unsigned char* p, size_t& count, string& op, string& operands) const
{
	const char* reg[] = { "AX", "BX", "CX", "DX", "SP", "BP", "SI", "DI" };
	if (*p >= 0x50 && *p <= 0x57) {
		op = "PUSH";
		operands = reg[*p - 0x50];
		count = 1;
	} else if (*p >= 0x58 && *p <= 0x5F) {
		op = "POP";
		operands = reg[*p - 0x58];
		count = 1;
	} else if (*p == 0x58) {
		op = "POP";
		operands = "AX";
		count = 1;
	} else {
		op = "DB";
		operands = Hex(*p);
		count = 1;
	}
	return true;
}

bool Memory::Unassemble(unsigned short seg, unsigned short offset) const
{
	for (unsigned short x = offset; x < offset + 32; ) {
		size_t count = 0;
		string op;
		string operands;
		ParseOneInstrument(&data_[seg * 16 + x], count, op, operands);

		string data;
		for (size_t i = 0; i < count; ++i) {
			data += Hex(data_[seg * 16 + x + i]);
		}
		printf("%04X:%04X %-14s%-8s%s\n", seg, x, data.c_str(), op.c_str(), operands.c_str());
		x += count;
	}
	return true;
}

void Processor::SetProcessorType(ProcessorType type)
{
	processor = type;
}

void Processor::SetCoProcessorType(CoProcessorType type)
{
	coprocessor = type;
}

void Processor::ShowProcessorType()
{
	switch (processor) {
	case ProcessorType::PT_8086: cout << "8086/88"; break;
	case ProcessorType::PT_186:  cout << "186";     break;
	case ProcessorType::PT_286:  cout << "286";     break;
	case ProcessorType::PT_386:  cout << "386";     break;
	case ProcessorType::PT_486:  cout << "486";     break;
	case ProcessorType::PT_586:  cout << "586";     break;
	case ProcessorType::PT_686:  cout << "686";     break;
	default: cout.setstate(ios_base::failbit);
	}

	cout << " ";

	switch (coprocessor) {
	case CoProcessorType::CPT_NONE: cout << "without coprocessor"; break;
	case CoProcessorType::CPT_287:  cout << "with 287";            break;
	case CoProcessorType::CPT_387:  cout << "with coprocessor";    break;
	default: cout.setstate(ios_base::failbit);
	}

	cout << endl;
}

void Registers::Dump()
{
	printf("AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n",
			regs_[AX], regs_[BX], regs_[CX], regs_[DX], regs_[SP], regs_[BP], regs_[SI], regs_[DI]);
	printf("DS=%04X  ES=%04X  SS=%04X  CS=%04X  IP=%04X   %s %s %s %s %s %s %s %s\n",
			regs_[DS], regs_[ES], regs_[SS], regs_[CS], regs_[IP],
			((regs_[FLAGS] & 0x0800) != 0 ? "OV" : "NV"), // Bit 11 (of): 1000 0000 0010 = 802 - Overflow (OV)
			((regs_[FLAGS] & 0x0400) != 0 ? "DN" : "UP"), // Bit 10 (df): 0100 0000 0010 = 402 - Direction Down (DN)
			((regs_[FLAGS] & 0x0200) != 0 ? "EI" : "DI"), // Bit  9 (if): 0010 0000 0010 = 202 - Interrupt Enabled (EI)
			((regs_[FLAGS] & 0x0080) != 0 ? "NG" : "PL"), // Bit  7 (sf): 0000 1000 0010 =  82 - Sign Negative (NG)
			((regs_[FLAGS] & 0x0040) != 0 ? "ZR" : "NZ"), // Bit  6 (zf): 0000 0100 0010 =  42 - Zero (ZR)
			((regs_[FLAGS] & 0x0010) != 0 ? "AC" : "NA"), // Bit  4 (af): 0000 0001 0010 =  12 - Auxiliary Carry (AC)
			((regs_[FLAGS] & 0x0004) != 0 ? "PE" : "PO"), // Bit  2 (pf): 0000 0000 0110 =  06 - Parity Even (PE)
			((regs_[FLAGS] & 0x0001) != 0 ? "CY" : "NC")  // Bit  0 (cf): 0000 0000 0011 =  03 - Carry (CY)
			);
	printf("%04X:%04X F60000        TEST    BYTE PTR [BX+SI],00                  DS:0000=CD\n", regs_[CS], regs_[IP]);
}

string ToUpper(const string& s)
{
	string r = s;
	for (auto& c : r) {
		c = toupper(c);
	}
	return r;
}

bool Registers::GetSeg(const string& name, unsigned short& value) const
{
	auto uname = ToUpper(name);
	if (uname == "DS") {
		value = regs_[DS];
	} else if (uname == "ES") {
		value = regs_[ES];
	} else if (uname == "SS") {
		value = regs_[SS];
	} else if (uname == "CS") {
		value = regs_[CS];
	} else {
		return false;
	}
	return true;
}

bool Registers::Get(const string& name, unsigned short& value) const
{
	auto uname = ToUpper(name);
	if (uname == "DS") {
		value = regs_[DS];
	} else if (uname == "ES") {
		value = regs_[ES];
	} else if (uname == "SS") {
		value = regs_[SS];
	} else if (uname == "CS") {
		value = regs_[CS];
	} else if (uname == "IP") {
		value = regs_[IP];
	} else if (uname == "AX") {
		value = regs_[AX];
	} else if (uname == "BX") {
		value = regs_[BX];
	} else if (uname == "CX") {
		value = regs_[CX];
	} else if (uname == "DX") {
		value = regs_[DX];
	} else if (uname == "SP") {
		value = regs_[SP];
	} else if (uname == "BP") {
		value = regs_[BP];
	} else if (uname == "SI") {
		value = regs_[SI];
	} else if (uname == "DI") {
		value = regs_[DI];
	} else {
		return false;
	}
	return true;
}

bool Registers::Set(const string& name, unsigned short value)
{
	auto uname = ToUpper(name);
	if (uname == "DS") {
		regs_[DS] = value;
	} else if (uname == "ES") {
		regs_[DS] = value;
	} else if (uname == "SS") {
		regs_[SS] = value;
	} else if (uname == "CS") {
		regs_[CS] = value;
	} else if (uname == "IP") {
		regs_[IP] = value;
	} else if (uname == "AX") {
		regs_[AX] = value;
	} else if (uname == "BX") {
		regs_[BX] = value;
	} else if (uname == "CX") {
		regs_[CX] = value;
	} else if (uname == "DX") {
		regs_[DX] = value;
	} else if (uname == "SP") {
		regs_[SP] = value;
	} else if (uname == "BP") {
		regs_[BP] = value;
	} else if (uname == "SI") {
		regs_[SI] = value;
	} else if (uname == "DI") {
		regs_[DI] = value;
	} else {
		return false;
	}
	return true;
}

int main(int argc, char* const* argv)
{
	vector<string> args(argv + 1, argv + argc);
	Processor processor;

	ConsoleUI ui;
	if (!ui.Init(processor.GetRegisters(), processor.GetMemory(), args)) {
		cerr << "Error: Failed to initialize console UI!" << endl;
		return 1;
	}

	for (;;) {
		auto cmd = ui.GetCommand();
		ui.Process(cmd, processor);
	}
	return 0;
}
