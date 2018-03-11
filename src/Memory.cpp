#include <cstdio>
#include <cstdlib>
#include "Memory.h"

Memory::Memory()
{
	srand(123);
	for (size_t i = 0; i < sizeof(memory); ++i) {
		memory[i] = static_cast<unsigned char>(rand());
	}
}

void Memory::Dump(unsigned short segment, unsigned short offset, unsigned short length)
{
	for (int i = 0; i < 8; ++i) {
		printf("%04X:%04X ", segment, cursor);
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

