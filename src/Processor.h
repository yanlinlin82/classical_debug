#ifndef PROCESSOR_H__
#define PROCESSOR_H__

#include <string>
#include "Registers.h"

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
	void ShowRegisters();

	unsigned short GetDataSegment() const { return registers_.GetDS(); }
private:
	ProcessorType processor{ProcessorType::PT_686};
	CoProcessorType coprocessor{CoProcessorType::CPT_387};
	Registers registers_;
};

#endif
