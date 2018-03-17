#ifndef REGISTERS_H__
#define REGISTERS_H__

#include <string>
#include <map>

class Registers
{
public:
	enum {
		FLAGS = 0,

		AX, BX, CX, DX, SP, BP, SI, DI,
		DS, ES, SS, CS, IP,

		MAX_REG_INDEX,
		MIN_REG_INDEX = 1
	};

	void Dump();

	unsigned short GetDS() const { return regs_[DS]; }
	bool GetSeg(const std::string& name, unsigned short& value) const;
	bool Get(const std::string& name, unsigned short& value) const;
	bool Set(const std::string& name, unsigned short value);
private:
	unsigned regs_[MAX_REG_INDEX] = {
		0, // FLAGS
		0, 0, 0, 0, 0xFFFE, 0, 0, 0, // AX, BX, CX, DX, SP, BP, SI, DI
		0x07BE, 0x07BE, 0x07BE, 0x07BE, 0x0100  // DS, ES, SS, CS, IP
	};
};

#endif
