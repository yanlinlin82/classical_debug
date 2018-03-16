#include <string>
#include <iostream>
#include "Processor.h"

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
	case ProcessorType::PT_8086: std::cout << "8086/88"; break;
	case ProcessorType::PT_186:  std::cout << "186";     break;
	case ProcessorType::PT_286:  std::cout << "286";     break;
	case ProcessorType::PT_386:  std::cout << "386";     break;
	case ProcessorType::PT_486:  std::cout << "486";     break;
	case ProcessorType::PT_586:  std::cout << "586";     break;
	case ProcessorType::PT_686:  std::cout << "686";     break;
	default: std::cout.setstate(std::ios_base::failbit);
	}

	std::cout << " ";

	switch (coprocessor) {
	case CoProcessorType::CPT_NONE: std::cout << "without coprocessor"; break;
	case CoProcessorType::CPT_287:  std::cout << "with 287";            break;
	case CoProcessorType::CPT_387:  std::cout << "with coprocessor";    break;
	default: std::cout.setstate(std::ios_base::failbit);
	}

	std::cout << std::endl;
}
