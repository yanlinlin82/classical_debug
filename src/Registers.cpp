#include <cstdio>
#include <locale>
#include "Registers.h"

void Registers::Dump()
{
	printf("AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n",
			regs_[AX], regs_[BX], regs_[CX], regs_[DX], regs_[SP], regs_[BP], regs_[SI], regs_[DI]);
	printf("DS=%04X  ES=%04X  SS=%04X  CS=%04X  IP=%04X   NV UP DI PL NZ NA PO NC\n",
			regs_[DS], regs_[ES], regs_[SS], regs_[CS], regs_[IP]);
	printf("%04X:%04X F60000        TEST    BYTE PTR [BX+SI],00                  DS:0000=CD\n", regs_[CS], regs_[IP]);
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
		value = regs_[DS];
	} else if (uname == "ES") {
		value = regs_[ES];
	} else if (uname == "SS") {
		value = regs_[SS];
	} else if (uname == "CS") {
		value = regs_[CS];
	} else {
		return false;
	}
	return true;
}

bool Registers::Get(const std::string& name, unsigned short& value) const
{
	auto uname = ToUpper(name);
	if (uname == "DS") {
		value = regs_[DS];
	} else if (uname == "ES") {
		value = regs_[ES];
	} else if (uname == "SS") {
		value = regs_[SS];
	} else if (uname == "CS") {
		value = regs_[CS];
	} else if (uname == "IP") {
		value = regs_[IP];
	} else if (uname == "AX") {
		value = regs_[AX];
	} else if (uname == "BX") {
		value = regs_[BX];
	} else if (uname == "CX") {
		value = regs_[CX];
	} else if (uname == "DX") {
		value = regs_[DX];
	} else if (uname == "SP") {
		value = regs_[SP];
	} else if (uname == "BP") {
		value = regs_[BP];
	} else if (uname == "SI") {
		value = regs_[SI];
	} else if (uname == "DI") {
		value = regs_[DI];
	} else {
		return false;
	}
	return true;
}

bool Registers::Set(const std::string& name, unsigned short value)
{
	auto uname = ToUpper(name);
	if (uname == "DS") {
		regs_[DS] = value;
	} else if (uname == "ES") {
		regs_[DS] = value;
	} else if (uname == "SS") {
		regs_[SS] = value;
	} else if (uname == "CS") {
		regs_[CS] = value;
	} else if (uname == "IP") {
		regs_[IP] = value;
	} else if (uname == "AX") {
		regs_[AX] = value;
	} else if (uname == "BX") {
		regs_[BX] = value;
	} else if (uname == "CX") {
		regs_[CX] = value;
	} else if (uname == "DX") {
		regs_[DX] = value;
	} else if (uname == "SP") {
		regs_[SP] = value;
	} else if (uname == "BP") {
		regs_[BP] = value;
	} else if (uname == "SI") {
		regs_[SI] = value;
	} else if (uname == "DI") {
		regs_[DI] = value;
	} else {
		return false;
	}
	return true;
}
