#ifndef MEMORY_H__
#define MEMORY_H__

class Memory
{
public:
	Memory();

	void Dump(unsigned short segment, unsigned short offset, unsigned short length);
private:
	unsigned char memory[65536];
	unsigned short cursor = 0x100;
};

#endif
