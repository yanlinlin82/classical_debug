#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Memory.h"

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

void Memory::PutData(unsigned short seg, unsigned short start, std::vector<unsigned char> data)
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
		const std::vector<unsigned char>& data)
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
		const std::vector<unsigned char>& data)
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

bool Memory::Load(const std::string& filename, unsigned short seg,
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
			memcpy(buf, &data_[realOffset], n);
			realOffset += n;
			size += n;
		}
	}
	fclose(fp);
	return true;
}

bool Memory::Write(const std::string& filename, unsigned short seg,
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
