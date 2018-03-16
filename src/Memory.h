#ifndef MEMORY_H__
#define MEMORY_H__

#include <vector>
#include <string>

class Memory
{
public:
	Memory();

	void Dump(unsigned short seg, unsigned short start, unsigned short end);

	void Compare(unsigned short srcSeg, unsigned short srcStart, unsigned short srcEnd,
			unsigned short dstSeg, unsigned short dstStart);
	void Copy(unsigned short srcSeg, unsigned short srcStart, unsigned short srcEnd,
			unsigned short dstSeg, unsigned short dstStart);
	void PutData(unsigned short srcSeg, unsigned short srcStart, std::vector<unsigned char> data);

	unsigned char GetChar(unsigned short seg, unsigned short offset) const;
	void SearchData(unsigned short seg, unsigned short start, unsigned short end,
			const std::vector<unsigned char>& data);
	void FillData(unsigned short seg, unsigned short start, unsigned short end,
			const std::vector<unsigned char>& data);

	bool Load(const std::string& filename, unsigned short seg,
			unsigned short offset, unsigned short& size);
	bool Write(const std::string& filename, unsigned short seg,
			unsigned short offset, unsigned short size);
private:
	std::vector<unsigned char> data_;
};

#endif
