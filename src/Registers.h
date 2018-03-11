#ifndef REGISTERS_H__
#define REGISTERS_H__

class Registers
{
public:
	void Dump();
	unsigned short GetDS() const { return ds; }
private:
	unsigned short ax = 0;
	unsigned short bx = 0;
	unsigned short cx = 0;
	unsigned short dx = 0;
	unsigned short sp = 0xFFFE;
	unsigned short bp = 0;
	unsigned short si = 0;
	unsigned short di = 0;
	unsigned short ds = 0x07BE;
	unsigned short es = 0x07BE;
	unsigned short ss = 0x07BE;
	unsigned short cs = 0x07BE;
	unsigned short ip = 0x0100;
	unsigned short flag = 0;
};

#endif
