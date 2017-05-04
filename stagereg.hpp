#ifndef STAGEREG_HPP
#define STAGEREG_HPP

// This file defines the state registers


struct IF_ID
{
	// actual stage register
	uint PCincremented; // store incremented pc value
	uint instruction; // store instruction from instruction memory
	uint immediate;
	uint rs;
	uint rt;
	uint rd;
};

//extern struct IF_ID ifid;
//extern struct IF_ID shadifid;

struct ID_EX
{
	//store instruction information
	uint WBreg;
	uint opcode; // store opcode
	uint rs; // store rs (first source reg #)
	uint rsVal;
	uint rt; // store rt (second source reg #)
	uint rtVal;
	uint rd; // store rd (destination reg #)
	uint shamt; // store shift amount
	uint funct; // store funct
	//uint immediateunsigned; // store immediate
	int16_t signedimmediate;
	uint target; // store branch target
	uint PCincremented; // store incremented program counter
	bool regDst;
	//bool ALUSrc; // never referenced
	bool memToReg;
	bool regWrite;
	bool memRead;
	bool memWrite;
	bool branch;
};

//extern struct ID_EX idex;
//extern struct ID_EX shadidex;

struct EX_MEM
{
	uint opcode;
	uint rt;
	uint WBreg;
	int dataOut;
	int ALUresult; // store alu result
	uint PCincremented; // store new program counter value
	bool memToReg;
	bool regWrite;
	bool memRead;
	bool memWrite;
	bool branch;
	uint8_t offset;
};

//extern struct EX_MEM exmem;
//extern struct EX_MEM shadexmem;

struct MEM_WB
{	
	int data; // store data
	int dataOut;
	uint WBreg; // store destination register
	uint PCincremented;
	bool memToReg;
	bool regWrite;
};

//extern struct MEM_WB memwb;
//extern struct MEM_WB shadmemwb;

#endif
