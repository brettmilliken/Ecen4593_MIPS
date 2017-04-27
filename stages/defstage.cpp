// define functions(procedures) for each stage
#include <cstdint>

#include "stagereg.hpp"
#include "instructions.hpp"
#include "pc.hpp"
//notes
// PC.address
// instructions[i].value
extern pc PC;
extern int gregisters[];
extern int memory[];
bool stallPipe;
int pcNext;
int graboffset;
int temp;
void stall()
{
	idex.rs = 0; // store rs (first source reg #)
	idex.rt = 0; // store rt (second source reg #)
	ifid.instruction = 0x00000000;
	idex.regDst = false;
	//shadidex.ALUSrc = false;
	idex.memToReg = false;
	idex.regWrite = false;
	idex.memRead = false;
	idex.memWrite = false;
	idex.branch = false;
	stallPipe = true;	
}
void setcontrol()
{
	//set control lines depending on opcode
	if(shadidex.opcode == 0x00 || 0x1F) {// r type instructions
		shadidex.regDst = true;
		//shadidex.ALUSrc = false;
		shadidex.memToReg = false;
		shadidex.regWrite = true;
		shadidex.memRead = false;
		shadidex.memWrite = false;
		shadidex.branch =  false;
	}
	if(shadidex.opcode == 0x20 || shadidex.opcode == 0x24 || shadidex.opcode == 0x25 ||
	shadidex.opcode == 0x0F || shadidex.opcode == 0x23 ){ //load instructions
		shadidex.regDst = false;
		//shadidex.ALUSrc = true;
		shadidex.memToReg = true;
		shadidex.regWrite = true;
		shadidex.memRead = true;
		shadidex.memWrite = false;
		shadidex.branch = false;
	}
	if(shadidex.opcode == 0x28 || shadidex.opcode == 0x29 || shadidex.opcode == 0x2B ){ 
		// store instructions
		shadidex.regDst = false; // don't care
		//shadidex.ALUSrc = true;
		shadidex.memToReg = false; // don't care
		shadidex.regWrite = false;
		shadidex.memRead = false;
		shadidex.memWrite = true;
		shadidex.branch = false;	
	}
	if(shadidex.opcode == 0x4 || shadidex.opcode == 0x5 || shadidex.opcode == 0x6 ||
	shadidex.opcode == 0x7 || shadidex.opcode == 0x1 || shadidex.opcode == 0x2 || shadidex.opcode == 0x3 || (shadidex.opcode == 0x00 && shadidex.funct == 0x8)){ // branch instructions
		shadidex.regDst = false; // don't care
		//shadidex.ALUSrc = false;
		shadidex.memToReg = false; // don't care
		shadidex.regWrite = false;
		shadidex.memRead = false;
		shadidex.memWrite = false;
		shadidex.branch = true;
	}
	else // immediate instructions
	{
		shadidex.regDst = false;
		//shadidex.ALUSrc = false;
		shadidex.memToReg = false;
		shadidex.regWrite = true;
		shadidex.memRead = false;
		shadidex.memWrite = false;
		shadidex.branch = false;
	}
}
/*
int signExtend(offset)
{
	if(offset & 0x00008000){
		return offset += 0xFFFF0000;
	}
}
*/
/* *****************************************************************************
8888888888 8888888888 88888888888 .d8888b.  888    888 
888        888            888    d88P  Y88b 888    888 
888        888            888    888    888 888    888 
8888888    8888888        888    888        8888888888 
888        888            888    888        888    888 
888        888            888    888    888 888    888 
888        888            888    Y88b  d88P 888    888 
888        8888888888     888     "Y8888P"  888    888                                      
******************************************************************************/

void IF()
{
	stallPipe = false;
	shadifid.instruction = instructions[PC.address].value; // store current instruction
	// increment pc value and store
	shadifid.PCincremented = PC.address + 1;
	pcNext = shadifid.PCincremented;
	shadifid.rs = (shadifid.instruction & 0x03E00000) >> 21; // store rs (first source reg #)
	shadifid.rt = (shadifid.instruction & 0x001F0000) >> 16; // store rt (second source reg #)
	shadifid.rd = (shadifid.instruction & 0x0000F800) >> 11; // store rd (destination reg #)

}

/* *****************************************************************************
8888888b.  8888888888 .d8888b.   .d88888b.  8888888b.  8888888888 
888  "Y88b 888       d88P  Y88b d88P" "Y88b 888  "Y88b 888        
888    888 888       888    888 888     888 888    888 888        
888    888 8888888   888        888     888 888    888 8888888    
888    888 888       888        888     888 888    888 888        
888    888 888       888    888 888     888 888    888 888        
888  .d88P 888       Y88b  d88P Y88b. .d88P 888  .d88P 888        
8888888P"  8888888888 "Y8888P"   "Y88888P"  8888888P"  8888888888  
******************************************************************************/

void ID()
{
	shadidex.opcode = (ifid.instruction & 0xFC000000) >> 26; // mask off opcode and shift
	setcontrol();
	if(shadidex.regDst)
	{
		shadidex.WBreg = ifid.rd;
	}
	else
	{
		shadidex.WBreg = ifid.rt;
	}
	
	// branch control hazards
	if(idex.regWrite && idex.WBreg != 0 && ((idex.WBreg == shadidex.rt) || (idex.WBreg == shadidex.rs)))
	{
		stall();
	}
	else if(exmem.regWrite && exmem.memRead && exmem.WBreg != 0 && ((exmem.WBreg == shadidex.rt) || (exmem.WBreg == shadidex.rs)))
	{
		stall();
	}
	// load hazard
	else if(idex.memRead && ((idex.rt == ifid.rs) || (idex.rt == ifid.rt)) && idex.WBreg != 0)
	{
		stall();
	}
	shadidex.rs = ifid.rs; // store rs (first source reg #)
	shadidex.rt = ifid.rt; // store rt (second source reg #)
	shadidex.rd = ifid.rd; // store rd (destination reg #)
	shadidex.rsVal = gregisters[shadidex.rs];
	shadidex.rtVal = gregisters[shadidex.rt];
	shadidex.shamt = (ifid.instruction & 0x000007C0) >> 6;
	shadidex.funct = (ifid.instruction & 0x0000003F);
	shadidex.immediate = (ifid.instruction & 0x0000FFFF);
	shadidex.target = (ifid.instruction & 0x03FFFFFF) ; // mask jump index

	if(shadidex.branch) // branch condition
	{
		switch(shadidex.opcode)	{
			case 0x4: // BEQ
				if(idex.rsVal == idex.rtVal) {
					pcNext = ifid.PCincremented + int(shadidex.immediate);
				}
				break;

			case 0x5: // BNE
				if(idex.rsVal != idex.rtVal) {
					pcNext = ifid.PCincremented + int(shadidex.immediate);
				}
				break;

			case 0x7: // BGTZ
				if(idex.rsVal > 0){
					pcNext = ifid.PCincremented + int(shadidex.immediate);
				}
				break;

			case 0x1: // BLTZ
				if(idex.rsVal < 0){
					pcNext = ifid.PCincremented + int(shadidex.immediate);
				}
				break;

			case 0x6: // BLEZ
				if(idex.rsVal <= 0){
					pcNext = ifid.PCincremented + int(shadidex.immediate);
				}
				break;

			case 0x2: // J
				pcNext = (0x3C0000000 & ifid.PCincremented) | (shadidex.target);
				break;

			case 0x3: // JAL
				gregisters[31] = (ifid.PCincremented + 1) << 2; // set return address
				pcNext = (0x3C0000000 & ifid.PCincremented) | (shadidex.target);
				break;
				
			case 0x0:
			switch(shadidex.funct) {
				case 0x8: // JR
					pcNext = gregisters[ifid.rs] >> 2;
					break;		
			}
			break;
		}
	}
}
/* *****************************************************************************
8888888888 Y88b   d88P 8888888888 .d8888b.  888     888 88888888888 8888888888 
888         Y88b d88P  888       d88P  Y88b 888     888     888     888        
888          Y88o88P   888       888    888 888     888     888     888        
8888888       Y888P    8888888   888        888     888     888     8888888    
888           d888b    888       888        888     888     888     888        
888          d88888b   888       888    888 888     888     888     888        
888         d88P Y88b  888       Y88b  d88P Y88b. .d88P     888     888        
8888888888 d88P   Y88b 8888888888 "Y8888P"   "Y88888P"      888     8888888888 
******************************************************************************/

void EX()
{

	// for R-type
	
	// rs is first source register for load instructions
	// rt is second source register
	
	// for load 
	
	shadexmem.opcode = idex.opcode; // set for load and store instructions.
	shadexmem.rtVal = idex.rtVal;
	
	// execution hazard
	if(exmem.regWrite && exmem.WBreg != 0 && idex.rs == exmem.WBreg) {  // Forwarding and hazards
		idex.rs = exmem.WBreg;
		idex.rsVal = exmem.ALUresult;
	}
	if(exmem.regWrite && exmem.WBreg != 0 && idex.rt == exmem.WBreg) {
		idex.rt = exmem.WBreg;
		idex.rtVal = exmem.ALUresult;
	}
	// memory hazard
	if(memwb.regWrite && memwb.WBreg != 0 && idex.rs == memwb.WBreg && !(exmem.regWrite && exmem.WBreg != 0 && idex.rs == exmem.WBreg)) {
		idex.rs = memwb.WBreg;
		idex.rsVal = memwb.data;
	}
	if(memwb.regWrite && memwb.WBreg != 0 && idex.rt == memwb.WBreg && !(exmem.regWrite && exmem.WBreg != 0 && idex.rt == exmem.WBreg)) {
		idex.rt = memwb.WBreg;
		idex.rtVal = memwb.data;
	}
	
	
	shadexmem.branch = idex.branch;
	shadexmem.memRead = idex.memRead;
	shadexmem.memWrite = idex.memWrite;
	shadexmem.regWrite = idex.regWrite;
	shadexmem.memToReg = idex.memToReg;
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
					if(uint32_t(idex.rsVal) < uint32_t(idex.rtVal)){
						shadexmem.ALUresult = 1;
					}
					break;

				case 0x0: // SLL
					shadexmem.ALUresult = idex.rtVal << idex.shamt;
					break;

				case 0x2: // SRL
					shadexmem.ALUresult = idex.rtVal >> idex.shamt;
					break;

				case 0x22: // SUB // implement trap on overflow (no trap yo)
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
					
				case 0x26: // XOR
					shadexmem.ALUresult = idex.rsVal ^ idex.rtVal;
					break;		
				}
            break;

		case 0x8: // ADDI // implement trap on overflow (haha jk)
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

        case 0xA: // SLTI
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

        case 0x20: // LB
            shadexmem.ALUresult = idex.rs + int(idex.immediate >> 2);
			graboffset = idex.immediate % 4;
            break;

        case 0x24: // LBU
            shadexmem.ALUresult = idex.rs + int(idex.immediate >> 2);
            graboffset = uint32_t(idex.immediate % 4);
            break;

        case 0x25: // LHU
			shadexmem.ALUresult = idex.rsVal + int(idex.immediate >> 2);
			graboffset = idex.immediate % 4;
            break;

        case 0xF: // LUI
            shadexmem.ALUresult = idex.immediate << 16;
            break;

        case 0x23: // LW
            shadexmem.ALUresult = int(idex.immediate >> 2) + idex.rsVal;
            break;

        case 0xD: // ORI
            shadexmem.ALUresult = idex.rsVal | (uint32_t(idex.immediate)) >> 16;
            break;

        case 0x28: // SB
			shadexmem.ALUresult = idex.rtVal + int(idex.immediate >> 2);
			graboffset = idex.immediate % 4;
            break;

        case 0x29: // SH
			shadexmem.ALUresult = idex.rtVal + int(idex.immediate >> 2);
			graboffset = idex.immediate % 4;
            break;

        case 0x2B: // SW
			shadexmem.ALUresult = idex.rtVal + int(idex.immediate >> 2);
            break;

        case 0x1F: // SEB
			shadexmem.ALUresult = int(idex.rtVal & 0xFF);
            break;
	}
}

/* *****************************************************************************
888b     d888 8888888888 888b     d888  .d88888b.  8888888b. Y88b   d88P 
8888b   d8888 888        8888b   d8888 d88P" "Y88b 888   Y88b Y88b d88P  
88888b.d88888 888        88888b.d88888 888     888 888    888  Y88o88P   
888Y88888P888 8888888    888Y88888P888 888     888 888   d88P   Y888P    
888 Y888P 888 888        888 Y888P 888 888     888 8888888P"     888     
888  Y8P  888 888        888  Y8P  888 888     888 888 T88b      888     
888   "   888 888        888   "   888 Y88b. .d88P 888  T88b     888     
888       888 8888888888 888       888  "Y88888P"  888   T88b    888     
******************************************************************************/

void MEM()
{	
	shadmemwb.WBreg = exmem.WBreg;
	shadmemwb.regWrite = exmem.regWrite;
	shadmemwb.memToReg = exmem.memToReg;
	shadmemwb.data = exmem.ALUresult;
	
	if(exmem.memWrite)
	{
		switch(exmem.opcode) // where is this done physically?
		{
			case 0x28: // SB
				temp = (0x000000FF & exmem.rtVal);
				switch(graboffset)
				{
					case 0:
						memory[exmem.ALUresult] = memory[exmem.ALUresult] & 0xFFFFFF00;
						break;
					
					case 1:
						memory[exmem.ALUresult] = memory[exmem.ALUresult] & 0xFFFF00FF;
						temp = temp << 8;
						break;
			
					case 2:
						memory[exmem.ALUresult] = memory[exmem.ALUresult] & 0xFF00FFFF;
						temp = temp << 16;
						break;
				
					case 3:
						memory[exmem.ALUresult] = memory[exmem.ALUresult] & 0x00FFFFFF;
						temp = temp << 24;
						break;
				}
				break;

			case 0x29: // SH
				temp = 0x0000FFFF & exmem.rtVal;
				switch(graboffset)
				{
					case 0:
						memory[exmem.ALUresult] = memory[exmem.ALUresult] & 0xFFFF0000;
						break;
			
					case 2:
						memory[exmem.ALUresult] = memory[exmem.ALUresult] & 0x0000FFFF;
						temp = temp << 16;
						break;
				}
				break;

			case 0x2B: // SW
				// Do nothing; Store word needs unmodified rtVal
				break;
		}
		memory[exmem.ALUresult] = memory[exmem.ALUresult] | temp;
	}
	if(exmem.memRead)
	{
		switch(exmem.opcode)
		{
			case 0x20: // LB
				switch(graboffset)
				{
					case 0:
						shadmemwb.dataOut = int((memory[exmem.ALUresult] & 0x000000FF));
						break;
					
					case 1:
						shadmemwb.dataOut = int((memory[exmem.ALUresult] & 0x0000FF00) >> 8);
						break;
			
					case 2:
						shadmemwb.dataOut = int((memory[exmem.ALUresult] & 0x00FF0000) >> 16);
						break;
				
					case 3:
						shadmemwb.dataOut = int((memory[exmem.ALUresult] & 0xFF000000) >> 24);
						break;
				}
				break;

			case 0x24: // LBU
				switch(graboffset)
				{
					case 0:
						shadmemwb.dataOut = uint32_t((memory[exmem.ALUresult] & 0x000000FF));
						break;
					
					case 1:
						shadmemwb.dataOut = uint32_t((memory[exmem.ALUresult] & 0x0000FF00) >> 8);
						break;
			
					case 2:
						shadmemwb.dataOut = uint32_t((memory[exmem.ALUresult] & 0x00FF0000) >> 16);
						break;
				
					case 3:
						shadmemwb.dataOut = uint32_t((memory[exmem.ALUresult] & 0xFF000000) >> 24);
						break;
				}
				break;

			case 0x25: // LHU
				switch(graboffset)
				{
					case 0:
						shadmemwb.dataOut = uint32_t((memory[exmem.ALUresult] & 0x0000FFFF));
						break;
								
					case 2:
						shadmemwb.dataOut = uint32_t((memory[exmem.ALUresult] & 0xFFFF0000) >> 16);
						break;
				
				}
				break;
			
			case 0xF: // LUI
				shadmemwb.dataOut = exmem.ALUresult;
				break;

			case 0x23: // LW
				shadmemwb.dataOut = memory[exmem.ALUresult];
				break;
		}
	}
}

/* ************************************************************************************************************
888       888 8888888b.  8888888 88888888888 8888888888           888888b.         d8888  .d8888b.  888    d8P  
888   o   888 888   Y88b   888       888     888                  888  "88b       d88888 d88P  Y88b 888   d8P   
888  d8b  888 888    888   888       888     888                  888  .88P      d88P888 888    888 888  d8P    
888 d888b 888 888   d88P   888       888     8888888     888888   8888888K.     d88P 888 888        888d88K     
888d88888b888 8888888P"    888       888     888                  888  "Y88b   d88P  888 888        8888888b    
88888P Y88888 888 T88b     888       888     888                  888    888  d88P   888 888    888 888  Y88b   
8888P   Y8888 888  T88b    888       888     888                  888   d88P d8888888888 Y88b  d88P 888   Y88b  
888P     Y888 888   T88b 8888888     888     8888888888           8888888P" d88P     888  "Y8888P"  888    Y88b                 
***************************************************************************************************************/

void WB()
{
	if(memwb.memToReg && memwb.regWrite)
	{
		gregisters[memwb.WBreg] = memwb.dataOut;
	}
	else if(memwb.regWrite)
	{
		gregisters[memwb.WBreg] = memwb.data;
	}
}

void shadToNorm()
{
	if(!stallPipe)
	{
		ifid = shadifid;
		PC.address = pcNext;
	}
	idex = shadidex;
	exmem = shadexmem;
	memwb = shadmemwb;
}
