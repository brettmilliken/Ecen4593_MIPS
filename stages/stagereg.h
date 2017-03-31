#ifndef STAGEREG_H
#define STAGEREG_H

// This file defines the state registers

struct IF_ID
{  
	int PCincremented = pc.address + 4; // store incremented pc value
	int instruction = mem[pc.address]; // store instruction from pc address location
};

struct ID_EX
{
	//store instruction information
	int opcode = (IF_ID.instruction & 0xFC000000) >> 26; // store opcode
	int rs = (IF_ID.instruction & 0x03E00000) >> 21; // store rs (first source reg #)
	int rt = (IF_ID.instruction & 0x001F0000) >> 16; // store rt (second source reg #)
	int rd = (IF_ID.instruction & 0x0000F800) >> 11; // store rd (destination reg #)
	int shamt = (IF_ID.instruction & 0x
	int funct;
	int immediate;
	int target;
	int PCincremented;
};

struct EX_MEM
{
	int ALUresult;
	int rd;
	int PCchanged;
};

struct MEM_WB
{
	int ALUresult;
	int data;
};

#endif
