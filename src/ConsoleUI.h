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

	void Process(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Processor& processor);
	void Process(const Command& cmd, Processor& processor);

	void ShowError(size_t space, const char* fmt, ...);
private:
	bool EnsureArgumentCount(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, size_t min, size_t max);

	void CompareMemory(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Registers& registers, Memory& memory);
	void CopyMemory(const std::vector<std::pair<size_t, std::string>>& words, Registers& registers, Memory& memory);
	void EnterData(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Registers& registers, Memory& memory);
	void SearchData(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Registers& registers, Memory& memory);
	void FillData(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Registers& registers, Memory& memory);

	void SetFilename(const std::string& f);
	void LoadData(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Registers& registers, Memory& memory);
	void WriteData(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize, Registers& registers, Memory& memory);
	void DumpMemory(const std::vector<std::pair<size_t, std::string>>& words, Registers& registers, Memory& memory);
	void SwitchProcessorType(const std::vector<std::pair<size_t, std::string>>& words, Processor& processor);
	void HexCalc(const std::vector<std::pair<size_t, std::string>>& words, size_t cmdSize);
	void ChangeRegisters(const std::vector<std::pair<size_t, std::string>>& words, Registers& registers);

	static void PrintUsage();

	enum NonBlock { NB_ENABLE, NB_DISABLE };
	static void SetNonBlock(NonBlock state);
	static int KeyboardHit();
	static int GetInputChar();
private:
	void ShowPrompt() const;
	std::string ReadLine();

	std::string prompt_ = "-";
	unsigned short curSeg_;
	unsigned short cursor_ = 0x100;

	std::string filename_ = "";
};

#endif
