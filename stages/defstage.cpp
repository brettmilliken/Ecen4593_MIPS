// define functions(procedures) for each stage

#include "stagereg.hpp"
#include "instructions.hpp"
#include "pc.hpp"
#include "stagereg.h"
//notes
// PC.address
// instructions[i].value
void stall(bool branch)
{
	if(branch) {
		
	}
}
void setcontrol()
{
	//set control lines depending on opcode
	if(shadidex.opcode == 0x00) {// r type instructions
		shadidex.regDst = true;
		shadidex.ALUSrc = false;
		shadidex.memToReg = false;
		shadidex.regWrite = true;
		shadidex.memRead = false;
		shadidex.memWrite = false;
		shadidex.branch = false;
	}
	if(shadidex.opcode == 0x20 || shadidex.opcode == 0x24 || shadidex.opcode == 0x25 ||
	shadidex.opcode == 0x0F || shadidex.opcode == 0x23 ){ //load instructions
		shadidex.regDst = false;
		shadidex.ALUSrc = true;
		shadidex.memToReg = true;
		shadidex.regWrite = true;
		shadidex.memRead = true;
		shadidex.memWrite = false;
		shadidex.branch = false;
	}
	if(shadidex.opcode == 0x28 || shadidex.opcode == 0x29 || shadidex.opcode == 0x2B ){ 
		// store instructions
		shadidex.regDst = false; // don't care
		shadidex.ALUSrc = true;
		shadidex.memToReg = false; // don't care
		shadidex.regWrite = false;
		shadidex.memRead = false;
		shadidex.memWrite = true;
		shadidex.branch = false;	
	}
	if(shadidex.opcode == 0x4 || shadidex.opcode == 0x5 || shadidex.opcode == 0x6 ||
	shadidex.opcode == 0x7 || shadidex.opcode == 0x1 ){ // branch instructions
		shadidex.regDst = false; // don't care
		shadidex.ALUSrc = false;
		shadidex.memToReg = false; // don't care
		shadidex.regWrite = false;
		shadidex.memRead = false;
		shadidex.memWrite = false;
		shadidex.branch = true;
	}
}

int signExtend(offset)
{
	if(offset & 0x00008000){
		return offset += 0xFFFF0000;
	}
}

void IF()
{
	shadifid.instruction = instructions[PC.address]; // store current instruction
	PC.address = PC.address + 1; // increment pc value and store
	shadifid.PCincremented = PC.address;
	shadifid.rs = (shadifid.instruction & 0x03E00000) >> 21; // store rs (first source reg #)
	shadifid.rt = (shadifid.instruction & 0x001F0000) >> 16; // store rt (second source reg #)
}

void ID()
{
	shadidex.opcode = (ifid.instruction & 0xFC000000) >> 26; // mask off opcode and shift
	setcontrol();
	shadidex.rs = ifid.rs // store rs (first source reg #)
	shadidex.rt = ifid.rt // store rt (second source reg #)
	shadidex.rd = (ifid.instruction & 0x0000F800) >> 11; // store rd (destination reg #)
	shadidex.rsVal = gregisters[shadidex.rs];
	shadidex.rtVal = gregisters[shadidex.rt];
	shadidex.shamt = (ifid.instruction & 0x000007C0) >> 6;
	shadidex.funct = (ifid.instruction & 0x0000003F);
	shadidex.immediate = (ifid.instruction & 0x0000FFFF);
	shadidex.target = (ifid.instruction & 0x0000FFFF); // calculate branch address
	shadidex.target = signExtend(shadidex.target);
	shadidex.jtarget = (ifid.instruction & 0x03FFFFFF) + (ifid.PCincremented & 0xF0000000);
	shadidex.PCincremented = ifid.PCincremented;
	if(idex.memRead && ((idex.rd == ifid.rs) || (idex.rd == ifid.rt)))
	{
		stall(true);
	}
	if(idex.memRead && ((exmem.WBreg == ifid.rs) || (exmem.WBreg == ifid.rt)))
	{
		stall(true);
	}
	if(idex.regWrite && !(idex.memRead) // what we checkin for? 
	switch(shadidex.opcode)	{
		case 0x4: // BEQ
			if(idex.rsVal == idex.rtVal) {
				shadexmem.ALUresult = idex.PCincremented + 4 + 4*idex.immediate;
			}
			break;

		case 0x5: // BNE
			if(idex.rsVal != idex.rtVal) {
				shadexmem.ALUresult = idex.PCincremented + 4 + 4*idex.immediate;
			}
			break;

		case 0x7: // BGTZ
			if(idex.rsVal > 0){
				shadexmem.ALUresult = at; // put in label addressing
			}
			break;

		case 0x1: // BLTZ
			if(idex.rsVal < 0){
				shadexmem.ALUresult = at; // put in label addressing
			}
			break;

		case 0x6: // BLEZ
			if(idex.rsVal <= 0){
				shadexmem.ALUresult = at; // put in label addressing
			}
			break;

		case 0x2: // J
			shadexmem.ALUresult = (0xF0000000 & idex.PCincremented) | (shadidex.target << 2);
            break;

		case 0x3: // JAL
            // SET gpr to PC + 2 (word aligned)
            // put jump and link stuff here
            break;

}

void EX()
{
	if(idex.regDst)
	{
		shadexmem.WBreg = idex.rd;
	}
	else
	{
		shadexmem.WBreg = idex.rt;
	}
	// for R-type
	
	// rs is first source register for load instructions
	// rt is second source register
	
	// for load 
	

	if(exmem.regWrite && exmem.WBreg != 0 && exmem.WBreg == idex.rs) {  // Forwarding and hazards
		idex.rs = exmem.WBreg;
		idex.rsVal = exmem.ALUresult;
	}
	if(exmem.regWrite && exmem.WBreg != 0 && exmem.WBreg == idex.rt) {
		idex.rt = exmem.WBreg;
		idex.rtVal = exmem.ALUresult;
	}
	if(memwb.regWrite && memwb.WBreg != 0 && memwb.WBreg == idex.rs && !(exmem.regWrite && exmem.WBreg != 0 && exmem.WBreg == idex.rs)) {
		idex.rs = memwb.WBreg;
		idex.rsVal = memwb.data;
	}
	if(memwb.regWrite && memwb.WBreg != 0 && memwb.WBreg == idex.rt && !(exmem.regWrite && exmem.WBreg != 0 && exmem.WBreg == idex.rt)) {
		idex.rt = memwb.WBreg;
		idex.rtVal = memwb.data;
	}
	// for load-use:
	if(idex.memRead && ((idex.rt == ifid.rs) || (idex.rt == ifid.rt)))
	{
		stall(false);
	}
	shadexmem.branch = idex.branch;
	shadexmem.memRead = idex.memRead;
	shadexmem.memWrite = idex.memWrite;
	shadexmem.regWrite = idex.regWrite;
	shadexmem.regDst = idex.regDst;
	shadexmem.memToReg = idex.memToReg;
	shadexmem.PCincremented = idex.PCincremented + 1;
	switch(idex.opcode) {
		case 0x00:
            switch(idex.funct) { // switch statement for r-type
				case 0x20: // ADD
					shadexmem.ALUresult = idex.rsVal + idex.rtVal;
					// add trap call if there is overflow
					// add overflow detection
					break;

				case 0x21: // ADDU
					shadexmem.ALUresult = idex.rsVal + idex.rtVal;
					break;
					
				case 0x24: // AND
					shadexmem.ALUresult = idex.rsVal & idex.rtVal;
					break;

				case 0x8: // JR
					shadexmem.ALUresult = shadexmem.target;
					break;

				case 0x27: // NOR
					shadexmem.ALUresult = ~(idex.rsVal | idex.rtVal);
					break;

				case 0x25: // OR
					shadexmem.ALUresult = idex.rsVal | idex.rtVal;
					break;

				case 0x2A: // SLT
					if(idex.rsVal < idex.rtVal){
						shadexmem.ALUresult = 1;
					}
					break;

				case 0x2B: // SLTU
					if(uint8(idex.rsVal) < idex.rtVal){
						shadexmem.ALUresult = 1;
					}
					break;

				case 0x0: // SLL
					shadexmem.ALUresult = idex.rtVal << shadexmem.shamt;
					break;

				case 0x2: // SRL
					shadexmem.ALUresult = idex.rtVal >> shadexmem.shamt;
					break;

				case 0x22: // SUB // implement trap on overflow
					shadexmem.ALUresult = idex.rsVal - idex.rtVal;
					break;

				case 0x23: // SUBU
					shadexmem.ALUresult = idex.rsVal - idex.rtVal;
					break;

				case 0xB: // MOVN
					if(idex.rtVal != 0){
						shadexmem.ALUresult = idex.rsVal;
					}
					break;

				case 0xA: // MOVZ
					if (idex.rtVal == 0){
						shadexmem.ALUresult = idex.rsVal;
					}
					break;

				}
            break;

		case 0x26: // XOR
			shadexmem.ALUresult = idex.rsVal ^ idex.rtVal;
			break;

		case 0x8: // ADDI // implement trap on overflow
			shadexmem.ALUresult = idex.rsVal + idex.immediate;
			break;

		case 0x9: // ADDIU
			shadexmem.ALUresult = idex.rsVal + idex.immediate;
			break;

		case 0xC: // ANDI
			shadexmem.ALUresult = idex.rsVal & (0x0000FFFF & idex.immediate);
			break;

		case 0xE: // XORI
			shadexmem.ALUresult = idex.rsVal ^ (0x0000FFFF & idex.immediate);
			break;

        case 0x20: // LB
            shadexmem.ALUresult = idex.rs + signExtend(idex.immediate);
            break;

        case 0x24: // LBU
            // to be done
            shadexmem.ALUresult = idex.rs + signExtend(idex.immediate);
            break;

        case 0x25: // LHU
            // to be done
			shadexmem.ALUresult = idex.rs + signExtend(idex.immediate);
            break;

        case 0xF: // LUI
            shadexmem.ALUresult = idex.immediate << 16;
            break;

        case 0x23: // LW
            shadexmem.ALUresult = signExtend(idex.immediate) + idex.rsVal;
            break;

        case 0xD: // ORI
            shadexmem.ALUresult = idex.rsVal | (uint32_t(idex.immediate)) >> 16;
            break;

        case 0xA: // SLTI
            shadexmem.rd = shadexmem.rt;
            if(idex.rsVal < idex.immediate){
                shadexmem.ALUresult = 1;
            }
            else
                shadexmem.ALUresult = 0;
            break;

        case 0xB: // SLTIU
            if(idex.rsVal < uint16_t(idex.immediate)){
                shadexmem.ALUresult = 1;
            }
            else
                shadexmem.ALUresult = 0;
            break;

        case 0x28: // SB
            // store byte to be done
            break;

        case 0x29: // SH
            // store halfword to be done
            break;

        case 0x2B: // SW
            // store word to be done
            shadexmem.rd = 
            break;

        case 0x1F: // SEB
            // sign-extend byte to be done
            break;
	}
}

void MEM()
{	
	shadmemwb.WBreg = exmem.WBreg;
	shadmemwb.regWrite = exmem.regWrite;
	shadmemwb.memToReg = exmem.memToReg;
	shadmemwb.data = exmem.ALUresult;
	if(exmem.memWrite)
	{
		memory[exmem.ALUresult] = exmem.rtVal;
	}
	if(exmem.memRead)
	{
		exmem.dataOut = memory[exmem.ALUresult];
	}
		if(shadidex.opcode == 0x20 || shadidex.opcode == 0x24 || shadidex.opcode == 0x25 ||
	shadidex.opcode == 0x0F || shadidex.opcode == 0x23 )//load
	if(shadidex.opcode == 0x28 || shadidex.opcode == 0x29 || shadidex.opcode == 0x2B )//store 	
}

void WB()
{
	gregisters[memwb.dest] = memwb.data;
}

void shadToNorm()
{
}
