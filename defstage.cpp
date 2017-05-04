// define functions(procedures) for each stage
#include <cstdint>
#include <iostream>
#include "stagereg.hpp"
//#include "instructions.hpp"
#include "pc.hpp"
#include "defstage.hpp"
#include "main_memory.hpp"
#include "SA_cache.hpp"
//#include "Load_Program.hpp"
//notes
// PC.address
// instructions[i].value

bool stallPipe;
int pcNext;
int temp;
int masktemp;
bool forward;
extern cache* dcache;
extern cache* icache;
extern struct pc PC;
int32_t signTemp;
IF_ID ifid;
IF_ID shadifid;
ID_EX idex;
ID_EX shadidex;
EX_MEM exmem;
EX_MEM shadexmem;
MEM_WB memwb;
MEM_WB shadmemwb;

void stall()
{
	//ifid.rs = 0; // store rs (first source reg #)
	//ifid.rt = 0; // store rt (second source reg #)
	//ifid.instruction = 0x00000000;
	shadidex.regDst = false;
	//shadidex.ALUSrc = false;
	shadidex.memToReg = false;
	shadidex.regWrite = false;
	shadidex.memRead = false;
	shadidex.memWrite = false;
	shadidex.branch = false;
	stallPipe = true;	
}

void setcontrol()
{
	//set control lines depending on opcode
	if((shadidex.opcode == 0x00) || (shadidex.opcode == 0x1F)) {
		// r type instructions
		shadidex.regDst = true;
		//shadidex.ALUSrc = false;
		shadidex.memToReg = false;
		shadidex.regWrite = true;
		shadidex.memRead = false;
		shadidex.memWrite = false;
		shadidex.branch =  false;
	}
	if((shadidex.opcode == 0x20) || (shadidex.opcode == 0x24) || (shadidex.opcode == 0x25) 
		|| (shadidex.opcode == 0x0F) || (shadidex.opcode == 0x23) ){ 
		//load instructions
		shadidex.regDst = false;
		//shadidex.ALUSrc = true;
		shadidex.memToReg = true;
		shadidex.regWrite = true;
		shadidex.memRead = true;
		shadidex.memWrite = false;
		shadidex.branch = false;
	}
	if((shadidex.opcode == 0x28) || (shadidex.opcode == 0x29) || (shadidex.opcode == 0x2B) ){ 
		// store instructions
		shadidex.regDst = false; // don't care
		//shadidex.ALUSrc = true;
		shadidex.memToReg = false; // don't care
		shadidex.regWrite = false;
		shadidex.memRead = false;
		shadidex.memWrite = true;
		shadidex.branch = false;	
	}
	if((shadidex.opcode == 0x4) || (shadidex.opcode == 0x5) || (shadidex.opcode == 0x6) 
		|| (shadidex.opcode == 0x7) || (shadidex.opcode == 0x1) || (shadidex.opcode == 0x2) 
		|| (shadidex.opcode == 0x3) || ((shadidex.opcode == 0x00) && (shadidex.funct == 0x8))){ 
		// branch instructions
		shadidex.regDst = false; // don't care
		//shadidex.ALUSrc = false;
		shadidex.memToReg = false; // don't care
		shadidex.regWrite = false;
		shadidex.memRead = false;
		shadidex.memWrite = false;
		shadidex.branch = true;
	}
	if((shadidex.opcode == 0x8)||(shadidex.opcode == 0x9)||(shadidex.opcode == 0xC)
		||(shadidex.opcode == 0xE)||(shadidex.opcode == 0xA)||(shadidex.opcode == 0xB)
		||(shadidex.opcode == 0xD)) // immediate instructions
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

int32_t signExtend(int offset)
{
	if(offset & 0x00008000){
		return (offset | 0xFFFF0000);
	}
	else {return (offset & 0x0000FFFF);}
}

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
	shadifid.instruction = icache->read(PC.address,true,cycle_count); // store current instruction
	// increment pc value and store
	shadifid.PCincremented = PC.address + 1;
	pcNext = shadifid.PCincremented;
	//std::cout << "inc PC: " << shadifid.PCincremented << "\n"; 
	//std::cout << "inst: " << std::hex << shadifid.instruction << "\n";
	shadifid.rs = (shadifid.instruction & 0x03E00000) >> 21; // store rs (first source reg #)
	shadifid.rt = (shadifid.instruction & 0x001F0000) >> 16; // store rt (second source reg #)
	shadifid.rd = (shadifid.instruction & 0x0000F800) >> 11; // store rd (destination reg #)
	shadifid.immediate = (shadifid.instruction & 0x0000FFFF);
	//std::cout << "rd: " << shadifid.rd << "\n";
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
	//std::cout << "pcNextDecode: " << std::dec << pcNext << "\n";
	shadidex.opcode = (ifid.instruction & 0xFC000000) >> 26; // mask off opcode and shift
	shadidex.rs = ifid.rs; // store rs (first source reg #)
	shadidex.rt = ifid.rt; // store rt (second source reg #)
	shadidex.rd = ifid.rd; // store rd (destination reg #)
	shadidex.rsVal = gregisters[shadidex.rs];
	shadidex.rtVal = gregisters[shadidex.rt];
	shadidex.shamt = (ifid.instruction & 0x000007C0) >> 6;
	shadidex.funct = (ifid.instruction & 0x0000003F);
	shadidex.signedimmediate = ifid.immediate;
	//shadidex.immediate = (int16_t)shadidex.immediateunsigned;
	shadidex.target = (ifid.instruction & 0x03FFFFFF); // mask jump index
	//std::cout << std::hex << "OP: " << shadidex.opcode << " FUNCT: " << shadidex.funct <<"\n";
	setcontrol();
	
	if(shadidex.regDst)
	{
		shadidex.WBreg = shadidex.rd;
	}
	else
	{
		shadidex.WBreg = shadidex.rt;
	}
	
	// branch forwarding
	// execution hazard
	if(shadidex.branch && (exmem.regWrite) && !(exmem.memRead) && (exmem.WBreg != 0) && (ifid.rs == exmem.WBreg)) {  // Forwarding and hazards
		shadidex.rsVal = exmem.ALUresult;
		forward = true;
	}
	if(shadidex.branch && (exmem.regWrite) && !(exmem.memRead) && (exmem.WBreg != 0) && (ifid.rt == exmem.WBreg)) {
		shadidex.rtVal = exmem.ALUresult;
		forward = true;
	}

	// branch control hazards
	
	if(shadidex.branch && (idex.regWrite) && (idex.WBreg != 0) 
		&& ((idex.WBreg == ifid.rt) || (idex.WBreg == ifid.rs)))
	{
		stall();
	}
	else if(shadidex.branch && !(forward) && (exmem.regWrite) && (exmem.memRead) && (exmem.WBreg != 0) 
		&& ((exmem.WBreg == ifid.rt) || (exmem.WBreg == ifid.rs)))
	{
		stall();
	}
	// load hazard
	if((idex.memRead) && ((idex.rt == ifid.rs) || (idex.rt == ifid.rt)) 
		&& (idex.WBreg != 0))
	{
		stall();
	}

	if(shadidex.branch) // branch detection
	{
		switch(shadidex.opcode)	{
			case 0x4: // BEQ
				if(int(shadidex.rsVal) == int(shadidex.rtVal)) {
					pcNext = ifid.PCincremented + shadidex.signedimmediate;
				}
				break;

			case 0x5: // BNE
				if(int(shadidex.rsVal) != int(shadidex.rtVal)) {
					pcNext = ifid.PCincremented + shadidex.signedimmediate;
				}
				break;

			case 0x7: // BGTZ
				if(int(shadidex.rsVal) > 0){
					pcNext = ifid.PCincremented + shadidex.signedimmediate;
				}
				break;

			case 0x1: // BLTZ
				if(int(shadidex.rsVal) < 0){
					pcNext = ifid.PCincremented + shadidex.signedimmediate;
				}
				break;

			case 0x6: // BLEZ
				if(int(shadidex.rsVal) <= 0){
					pcNext = ifid.PCincremented + shadidex.signedimmediate;
				}
				break;

			case 0x2: // J
				pcNext = ((0xF0000000 & (ifid.PCincremented << 2)) | (shadidex.target << 2)) >> 2;
				break;

			case 0x3: // JAL
				gregisters[31] = (ifid.PCincremented + 1) << 2; // set return address
				pcNext = ((0xF0000000 & (ifid.PCincremented << 2)) | (shadidex.target << 2)) >> 2;
				break;
			
			case 0x0: // JR
				if(shadidex.funct == 0x8) { // make sure JR
					pcNext = shadidex.rsVal >> 2;
				}
				break;	
			//std::cout << std::hex << "branch(taken) address: " << pcNext << "\n";
		}
	}
	forward = false;
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
	
	shadexmem.WBreg = idex.WBreg; 
	shadexmem.opcode = idex.opcode; // set for load and store instructions.
	
	// execution hazard
	if((exmem.regWrite) && (exmem.WBreg != 0) && (idex.rs == exmem.WBreg)) {  // Forwarding and hazards
		//idex.rsVal = exmem.ALUresult;
		gregisters[idex.rs] = exmem.ALUresult;
	}
	if((exmem.regWrite) && (exmem.WBreg != 0) && (idex.rt == exmem.WBreg)) {
		//idex.rtVal = exmem.ALUresult;
		gregisters[idex.rt] = exmem.ALUresult;
	}
	// memory hazard
	if((memwb.regWrite) && (memwb.WBreg != 0) && (idex.rs == memwb.WBreg) 
		&& !((exmem.regWrite) && (exmem.WBreg != 0) && (idex.rs == exmem.WBreg))) {
		if(memwb.memToReg){
			gregisters[idex.rs] = memwb.dataOut;
		}
		else
			gregisters[idex.rs] = memwb.data;

		//idex.rsVal = memwb.data;
		//std::cout <<"forwarded"<<"\n";
	}
	if((memwb.regWrite) && (memwb.WBreg != 0) && (idex.rt == memwb.WBreg) 
		&& !((exmem.regWrite) && (exmem.WBreg != 0) && (idex.rt == exmem.WBreg))) {
		if(memwb.memToReg){
			gregisters[idex.rt] = memwb.dataOut;
		}
		else
			gregisters[idex.rt] = memwb.data;

		//idex.rtVal = memwb.data;
	}
	
		//idex.rtVal = memory[memwb.data];
		//gregisters[idex.rt] = memory[memwb.data];
	
	shadexmem.rt = idex.rt;
	idex.rsVal = gregisters[idex.rs];
	idex.rtVal = gregisters[idex.rt];

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
					if(int32_t(idex.rsVal) < int32_t(idex.rtVal)){
						shadexmem.ALUresult = 1;
					}
					else
						shadexmem.ALUresult = 0;
					break;

				case 0x2B: // SLTU
					if(uint32_t(idex.rsVal) < uint32_t(idex.rtVal)){
						shadexmem.ALUresult = 1;
					}
					else
						shadexmem.ALUresult = 0;
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
					else
						shadexmem.regWrite = false;
					break;

				case 0xA: // MOVZ
					if (idex.rtVal == 0){
						shadexmem.ALUresult = idex.rsVal;
					}
					else
						shadexmem.regWrite = false;
					break;
					
				case 0x26: // XOR
					shadexmem.ALUresult = idex.rsVal ^ idex.rtVal;
					break;		
				}
            break;

		case 0x8: // ADDI // implement trap on overflow (haha jk)
			shadexmem.ALUresult = int(idex.rsVal) + idex.signedimmediate;
			break;

		case 0x9: // ADDIU
			shadexmem.ALUresult = int(idex.rsVal) + idex.signedimmediate;
			break;

		case 0xC: // ANDI
			shadexmem.ALUresult = idex.rsVal & (0x0000FFFF & uint(idex.signedimmediate));
			break;

		case 0xE: // XORI
			shadexmem.ALUresult = idex.rsVal ^ (0x0000FFFF & uint(idex.signedimmediate));
			break;

        case 0xA: // SLTI
            if(int(idex.rsVal) < idex.signedimmediate){
                shadexmem.ALUresult = 1;
            }
            else
                shadexmem.ALUresult = 0;
            break;

        case 0xB: // SLTIU
            if(idex.rsVal < uint16_t(idex.signedimmediate)){
                shadexmem.ALUresult = 1;
            }
            else
                shadexmem.ALUresult = 0;
            break;

        case 0x20: // LB
            shadexmem.ALUresult = idex.rsVal + idex.signedimmediate;
            break;

        case 0x24: // LBU
            shadexmem.ALUresult = idex.rsVal + idex.signedimmediate;
            break;

        case 0x25: // LHU
			shadexmem.ALUresult = idex.rsVal + idex.signedimmediate;
            break;

        case 0xF: // LUI
            shadexmem.ALUresult = idex.signedimmediate << 16;
            break;

        case 0x23: // LW
            shadexmem.ALUresult = idex.signedimmediate + idex.rsVal;
            break;

        case 0xD: // ORI
            shadexmem.ALUresult = idex.rsVal | (uint32_t(idex.signedimmediate));
            break;

        case 0x28: // SB
			shadexmem.ALUresult = idex.rsVal + idex.signedimmediate;
            break;

        case 0x29: // SH
			shadexmem.ALUresult = idex.rsVal + idex.signedimmediate;
            break;

        case 0x2B: // SW
			shadexmem.ALUresult = idex.rsVal + idex.signedimmediate;
            break;

        case 0x1F: // SEB
			signTemp = signExtend(idex.rtVal & 0xFF);
			shadexmem.ALUresult = signTemp;
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
	uint8_t offset = exmem.ALUresult%4;
	int storetemp;
	int wordaddress;

	if(exmem.memWrite)
	{
		switch(exmem.opcode) // where is this done physically?
		{
			case 0x28: // SB
				temp = (0x000000FF & gregisters[exmem.rt]);
				switch(offset)
				{
					case 0:
						wordaddress = (exmem.ALUresult >> 2);
						masktemp = dcache->read(wordaddress,false,cycle_count);
						masktemp = masktemp & 0xFFFFFF00;
						storetemp = masktemp | temp;
						dcache->write(wordaddress,storetemp,cycle_count);
						break;
					
					case 1:
						wordaddress = (exmem.ALUresult >> 2);
						masktemp = dcache->read(wordaddress,false,cycle_count) & 0xFFFF00FF;
						temp = temp << 8;
						storetemp = masktemp | temp;	
						dcache->write(wordaddress,storetemp,cycle_count);
						break;
			
					case 2:
						wordaddress = (exmem.ALUresult >> 2);
						masktemp = dcache->read(wordaddress,false,cycle_count) & 0xFF00FFFF;
						temp = temp << 16;
						storetemp = masktemp | temp;	
						dcache->write(wordaddress,storetemp,cycle_count);				
						break;
				
					case 3:
						wordaddress = (exmem.ALUresult >> 2);
						masktemp = dcache->read(wordaddress,false,cycle_count) & 0x00FFFFFF;
						temp = temp << 24;
						storetemp = masktemp | temp;
						dcache->write(wordaddress,storetemp,cycle_count);
						break;
				}
				break;

			case 0x29: // SH
				temp = 0x0000FFFF & gregisters[exmem.rt];
				switch(offset)
				{
					case 0:
						wordaddress = (exmem.ALUresult >> 2);
						masktemp = dcache->read(wordaddress,false,cycle_count) & 0xFFFF0000;
						storetemp = masktemp | temp;
						dcache->write(wordaddress,storetemp,cycle_count);
						break;
			
					case 2:
						wordaddress = (exmem.ALUresult >> 2);
						masktemp = dcache->read(wordaddress,false,cycle_count) & 0x0000FFFF;
						temp = temp << 16;
						storetemp = masktemp | temp;
						dcache->write(wordaddress,storetemp,cycle_count);
						break;
				}
				break;

			case 0x2B: // SW
				// Do nothing as entire word will be stored
				wordaddress = (exmem.ALUresult >> 2);
				storetemp = gregisters[exmem.rt];
				dcache->write(wordaddress,storetemp,cycle_count);
				break;
		}
	}
	if(exmem.memRead)
	{
		switch(exmem.opcode)
		{
			case 0x20: // LB
				switch(offset)
				{
					case 0:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = int32_t((dcache->read(wordaddress,true,cycle_count) & 0x000000FF));
						break;
					
					case 1:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = int32_t((dcache->read(wordaddress,true,cycle_count) & 0x0000FF00) >> 8);
						break;
			
					case 2:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = int32_t((dcache->read(wordaddress,true,cycle_count) & 0x00FF0000) >> 16);
						break;
				
					case 3:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = int32_t((dcache->read(wordaddress,true,cycle_count) & 0xFF000000) >> 24);
						break;
				}
				break;

			case 0x24: // LBU
				switch(offset)
				{
					case 0:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = uint32_t((dcache->read(wordaddress,true,cycle_count) & 0x000000FF));
						break;
					
					case 1:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = uint32_t((dcache->read(wordaddress,true,cycle_count) & 0x0000FF00) >> 8);
						break;
			
					case 2:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = uint32_t((dcache->read(wordaddress,true,cycle_count) & 0x00FF0000) >> 16);
						break;
				
					case 3:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = uint32_t(dcache->read(wordaddress,true,cycle_count) >> 24);
						break;
				}
				break;

			case 0x25: // LHU
				switch(offset)
				{
					case 0:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = uint32_t((dcache->read(wordaddress,true,cycle_count) & 0x0000FFFF));
						break;
								
					case 2:
						wordaddress = (exmem.ALUresult >> 2);
						shadmemwb.dataOut = uint32_t((dcache->read(wordaddress,true,cycle_count) & 0xFFFF0000) >> 16);
						break;
				
				}
				break;
			
			case 0xF: // LUI
				shadmemwb.dataOut = exmem.ALUresult;
				break;

			case 0x23: // LW
				wordaddress = (exmem.ALUresult >> 2);
				shadmemwb.dataOut = dcache->read(wordaddress,true,cycle_count);
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
	if(memwb.memToReg && memwb.regWrite && (memwb.WBreg != 0))
	{
		gregisters[memwb.WBreg] = memwb.dataOut;
	}
	if(memwb.regWrite && !memwb.memToReg && (memwb.WBreg != 0))
	{
		gregisters[memwb.WBreg] = memwb.data;
	}
}

void shadToNorm(int *iCount)
{
	if(!stallPipe)
	{
		ifid = shadifid;
		//std::cout << "pcNextShadToNorm: " << std::dec << pcNext << "\n";
		PC.address = pcNext;
		*iCount = *iCount + 1;
	}
	idex = shadidex;
	exmem = shadexmem;
	memwb = shadmemwb;
}
