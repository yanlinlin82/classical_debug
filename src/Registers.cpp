#include <cstdio>
#include <locale>
#include "Registers.h"

void Registers::Dump()
{
	printf("AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n",
			ax, bx, cx, dx, sp, bp, si, di);
	printf("DS=%04X  ES=%04X  SS=%04X  CS=%04X  IP=%04X   NV UP DI PL NZ NA PO NC\n",
			ds, es, ss, cs, ip);
	printf("%04X:%04X F60000        TEST    BYTE PTR [BX+SI],00                  DS:0000=CD\n", cs, ip);
}

std::string ToUpper(const std::string& s)
{
	std::string r = s;
	for (auto& c : r) {
		c = std::toupper(c);
	}
	return r;
}

bool Registers::GetSeg(const std::string& name, unsigned short& value) const
{
	auto uname = ToUpper(name);
	if (uname == "DS") {
		value = ds;
	} else if (uname == "ES") {
		value = es;
	} else if (uname == "SS") {
		value = ss;
	} else if (uname == "CS") {
		value = cs;
	} else {
		return false;
	}
	return true;
}
