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

	void Process(const Command& cmd, Processor& processor);
private:
	void ShowError(size_t space, const char* fmt, ...);
	bool EnsureArgumentCount(const Command& cmd, size_t min, size_t max);

	void CompareMemory(const Command& cmd, Registers& registers, Memory& memory);
	void CopyMemory(const Command& cmd, Registers& registers, Memory& memory);
	void EnterData(const Command& cmd, Registers& registers, Memory& memory);
	void SearchData(const Command& cmd, Registers& registers, Memory& memory);
	void FillData(const Command& cmd, Registers& registers, Memory& memory);

	void SetFilename(const std::string& f);
	void LoadData(const Command& cmd, Registers& registers, Memory& memory);
	void WriteData(const Command& cmd, Registers& registers, Memory& memory);
	void DumpMemory(const Command& cmd, Registers& registers, Memory& memory);
	void SwitchProcessorType(const Command& cmd, Processor& processor);
	void HexCalc(const Command& cmd);
	void ChangeRegisters(const Command& cmd, Registers& registers);

	static void PrintUsage();
private:
	void ShowPrompt() const;
	std::string ReadLine();

	std::string prompt_ = "-";
	unsigned short curSeg_;
	unsigned short cursor_ = 0x100;

	std::string filename_ = "";
};

#endif
