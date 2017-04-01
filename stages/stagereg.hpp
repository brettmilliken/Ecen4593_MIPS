#ifndef STAGEREG_HPP
#define STAGEREG_HPP

// This file defines the state registers

struct IF_ID
{  

	//shadow(temp) register
	int shadPCincremented; // store incremented pc value
	int shadinstruction; // store instruction from instruction memory
	
	// actual stage register
	int PCincremented; // store incremented pc value
	int instruction; // store instruction from instruction memory
};

extern struct IF_ID ifid;

struct ID_EX
{
	//store instruction information

	//shadow(temp) register
	int shadopcode; // store opcode
	int shadrs; // store rs (first source reg #)
	int shadrt; // store rt (second source reg #)
	int shadrd; // store rd (destination reg #)
	int shadshamt; // store shift amount
	int shadfunct; // store funct
	int shadimmediate; // store immediate
	int shadtarget; // store branch target
	int shadPCincremented; // store incremented program counter

	// actual stage register
	int opcode; // store opcode
	int rs; // store rs (first source reg #)
	int rt; // store rt (second source reg #)
	int rd; // store rd (destination reg #)
	int shamt; // store shift amount
	int funct; // store funct
	int immediate; // store immediate
	int target; // store branch target
	int PCincremented; // store incremented program counter
};

extern struct ID_EX idex;

struct EX_MEM
{
	//shadow(temp) register
	int shadALUresult; // store alu result
	int shadrd; // store destination reg #
	int shadPCchanged; // store new program counter value
		
	// actual stage register
	int ALUresult; // store alu result
	int rd; // store destination reg #
	int PCchanged; // store new program counter value
};

extern struct EX_MEM exmem;

struct MEM_WB
{
	//shadow(temp) register
	int shadALUresult; // store alu result
	int shaddata; // store data

	// actual stage register
	int ALUresult; // store alu result
	int data; // store data
};

extern struct MEM_WB memwb;

#endif
