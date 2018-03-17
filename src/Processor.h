#ifndef PROCESSOR_H__
#define PROCESSOR_H__

#include <string>
#include "Registers.h"
#include "Memory.h"

enum class ProcessorType
{
	PT_8086 = 0,
	PT_186,
	PT_286,
	PT_386,
	PT_486,
	PT_586,
	PT_686
};

enum class CoProcessorType
{
	CPT_NONE = 0,
	CPT_287,
	CPT_387
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

#endif
