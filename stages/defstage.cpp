// define functions(procedures) for each stage

#include "stagereg.hpp"
#include "instructions.hpp"
#include "pc.hpp"
#include "stagereg.h"
//notes
// PC.address
// instructions[i].value

void IF()
{
	ifid.shadinstruction = instructions[PC.address]; // store current instruction
	PC.address = PC.address + 1; // increment pc value and store
	ifid.shadPCincremented = PC.address
}

void ID()
{
	idex.shadopcode = (ifid.instruction & 0xFC000000) >> 26; // mask off opcode and shift
        idex.shadrs = (ifid.instruction & 0x03E00000) >> 21; // store rs (first source reg #)
        idex.shadrt = (ifid.instruction & 0x001F0000) >> 16; // store rt (second source reg #)
        idex.shadrd = (ifid.instruction & 0x0000F800) >> 11; // store rd (destination reg #)
        idex.shadshamt = (ifid.instruction & 0x000007C0) >> 6;
        idex.shadfunct = (ifid.instruction & 0x0000003F);
        idex.shadimmediate = (ifid.instruction & 0x0000FFFF);
        idex.shadtarget = (ifid.instruction & 0x03FFFFFF);
        idex.shadPCincremented = ifid.instruction.PCincremented;
	
}

void EX()
{
	exmem.shadrd = idex.rd;
	exmem.shadBranchTarget = idex.target;
	exmem.shadPCchanged = idex.PCincremented + 1;
	switch(idex.opcode) {
		case 0x00000000:; // NOP
		case 0x20: // ADD
			exmem.shadALUresult = idex.rs + idex.rt;
			// add trap call if there is overflow
			// add overflow detection
			break;
		case 0x21: // ADDU
			exmem.shadALUresult = idex.rs + idex.rt;
			break;
		case 0x24: // AND
			exmem.shadALUresult = idex.rs & idex.rt;
			break;
		case 0x8: // JR
			exmem.shadBranchTarget = idex.target;
			break;
		case 0x27: // NOR
			exmem.shadALUresult = ~(idex.rs|idex.rt);
			break;
		case 0x25: // OR
			exmem.shadALUresult = idex.rs | idex.rt;
			break;
		case 0x // put in opcode // MOVN
			if(idex.rt != 0){
				exmem.shadALUresult = idex.rs;
			}
			break;
		case 0x // put in opcode // MOVZ
			if (idex.rt == 0){
				exmem.shadALUresult = idex.rs;
			}
		case 0x2A: // SLT
			if(idex.rs < idex.rt){
				exmem.shadALUresult = 1;
			}
			break;
		case 0x2B: // SLTU
			if(uint8(idex.rs) < idex.rt){
				exmem.shadALUresult = 1;
			}
			break;
		case 0x00: // SLL
			exmem.shadALUresult = idex.rt << idex.shamt;
			break;
		case 0x2: // SRL
			exmem.shadALUresult = idex.rt >> idex.shamt;
			break;		
		case 0x22: // SUB // implement trap on overflow
			exmem.shadALUresult = idex.rs - idex.rt;
			break;
		case 0x23: // SUBU
			exmem.shadALUresult = idex.rs - idex.rt;
			break;
		case 0x26: // XOR
			exmem.shadALUresult = idex.rs ^ idex.rt;
			break;
		case 0x8: // ADDI // implement trap on overflow
			exmem.shadALUresult = idex.rs + idex.immediate;
			exmem.shadrd = idex.rt;
			break;
		case 0x9: // ADDIU
			exmem.shadALUresult = idex.rs + idex.immediate;
			exmem.shadrd = idex.rt;
			break;
		case 0xC: // ANDI
			exmem.shadALUresult = idex.rs & (0x0000FFFF & idex.immediate);
			exmem.shadrd = idex.rt;
			break;
		case 0xE: // XORI
			exmem.shadALUresult = idex.rs ^ (0x0000FFFF & idex.immediate);
			exmem.shadrd = idex.rt;
			break;
		case 0x4: // BEQ
			if(idex.rs == idex.rt) {
				exmem.shadBranchTarget = idex.PCincremented + 4 + 4*idex.immediate;
			
			}
			break;
		case 0x5: // BNE
			if(idex.rs != idex.rt) {
				exmem.shadBranchTarget = idex.PCincremented + 4 + 4*idex.immediate;
			}
			break;
		case 0x // bgtz
			if(idex.rs > 0){
				exmem.shadBranchTarget = at; // put in label addressing
			}
			break;
		case 0x // bltz
			if(idex.rs < 0){
				exmem.shadBranchTarget = at; // put in label addressing
			}
			break
		case 0x // blez
			if(idex.rs <= 0){
				exmem.shadBranchTarget = at; // put in label addressing
			}
		case 0x2: // j
			exmem.shadBranchTarget = (0xF0000000 & idex.PCincremented) | (idex.shadtarget << 2);
		case 0x

	}		
}

void MEM()
{
}

void WB()
{
}


