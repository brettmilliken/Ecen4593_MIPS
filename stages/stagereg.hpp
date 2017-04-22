#ifndef STAGEREG_HPP
#define STAGEREG_HPP

// This file defines the state registers


struct IF_ID
{
	// actual stage register
	int PCincremented; // store incremented pc value
	int instruction; // store instruction from instruction memory
	int rs;
	int rt;
};

extern struct IF_ID ifid;
extern struct IF_ID shadifid;

struct ID_EX
{
	//store instruction information
	int opcode; // store opcode
	int rs; // store rs (first source reg #)
	int rsVal;
	int rt; // store rt (second source reg #)
	int rtVal;
	int rd; // store rd (destination reg #)
	int shamt; // store shift amount
	int funct; // store funct
	int immediate; // store immediate
	int target; // store branch target
	int jtarget; // store jump target
	int PCincremented; // store incremented program counter
	bool regDst;
	bool ALUSrc;
	bool memToReg;
	bool regWrite;
	bool memRead;
	bool memWrite;
	bool branch;
	int WBreg;
};

extern struct ID_EX idex;
extern struct ID_EX shadidex;

struct EX_MEM
{
	int funct;
	int WBreg;
	int opcode;
	int rsVal;
	int rtVal;
	int ALUresult; // store alu result
	int PCchanged; // store new program counter value
	bool memToReg;
	bool regWrite;
	bool memRead;
	bool memWrite;
	bool branch;
};

extern struct EX_MEM exmem;
extern struct EX_MEM shadexmem;

struct MEM_WB
{	
	int data; // store data
	int WBreg; // store destination register
	int PC;
	bool memToReg;
	bool regWrite;
};

extern struct MEM_WB memwb;
extern struct MEM_WB shadmemwb;

#endif
