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
	PC.address = PC.address + 4; // increment pc value and store
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
}

void MEM()
{
}

void WB()
{
}


