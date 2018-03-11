#ifndef MEMORY_H__
#define MEMORY_H__

#include <vector>

class Memory
{
public:
	Memory();

	void Dump(unsigned short seg, unsigned short start, unsigned short end);

	void Compare(unsigned short srcSeg, unsigned short srcStart, unsigned short srcEnd,
			unsigned short dstSeg, unsigned short dstStart);
private:
	std::vector<unsigned char> data_;
};

#endif
