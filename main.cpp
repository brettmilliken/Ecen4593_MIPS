#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
//#include "instructions.hpp" 
#include "pc.hpp" 
#include "Load_Program.hpp"
#include "defstage.hpp" 
//#include "stagereg.hpp"

using namespace std;

// define global variables
//instr instructions[1000]; // define instruction memory
int sp = 29;
int fp = 30;
int iCount = 0;
int cycle_count = 0;
pc PC; // define program counter
//extern struct IF_ID ifid; // if/id stage register
//extern struct IF_ID shadifid;
//extern struct ID_EX idex; // id/ex stage register
//extern struct ID_EX shadidex;
//extern struct EX_MEM exmem; // ex/mem stage register
//extern struct EX_MEM shadexmem;
//extern struct MEM_WB memwb; // mem/wb stage register
//extern struct MEM_WB shadmemwb;
extern int memory[];
int gregisters[32];
//int memory[1200];
// end define global variables

int main(void)
{

	Initialize_Simulation_Memory();
  /* Infinite loop */
  while (1) {

	  /* initialize state elements */

	  PC.address = memory[5];
	  gregisters[sp] = memory[0];
	  gregisters[fp] = memory[1];
	  cycle_count = 0;
	  
	  while (PC.address != 0x00000000){
		  //cout<< "first inst: " << std::hex << memory[140]; 
		  //cout<< "PC Address before IF: " << std::dec << PC.address << "\n";
		  IF();
		  //cout<< "PC Address before WB: " << PC.address << "\n";
		  WB();
		  //cout<< "PC Address before ID: " << PC.address << "\n";
		  ID();
		  //cout<< "PC Address before EX: " << PC.address << "\n";
		  EX();
		  //cout<< "PC Address before MEM: " << PC.address << "\n";
		  MEM();
		  //cout<< "PC Address post mem end cycle: " << PC.address << "\n";
		  //cout << "instruction count: " << iCount << "\n";
		  //cout << "cycle count: " << cycle_count << "\n";
		  shadToNorm(&iCount);
		  //cout<< "PC Address post mem end cycle: " << PC.address << "\n";
		  cycle_count++;
		  /*cout<<"Cycle Count: " << cycle_count << "\n";
		  if(cycle_count >= 365983) {
			  for(int k = 235; k<600; k++){
					cout << "memory location: " << k << "data: " << memory[k] << "\n";
				  }
		break;
			  }*/
		  
	  }
	  //cout<< "PC Address post while: " << PC.address << "\n";
	  cout << "Desired: " << 112 << " " <<"Actual: " << memory[6] << "\n";
	  cout << "Desired: " << 29355 << " " << "Actual: " << memory[7] << "\n";
	  cout << "Desired: " << 14305 << " " << "Actual: " << memory[8] << "\n";
	  cout << "Desired: " << 0 << " " << "Actual: " << memory[9] << "\n";
	  /*	for(int k = 243; k<500; k++){
			cout << std::dec <<"memory location: " << k << std::hex << " data: " << memory[k] << "\n";
		}*/

	  //cout << "instruction count end: " << iCount << "\n";
	  //cout << "cycle count end: " << cycle_count << "\n";	 
	  break; 
  }
}
  


		  //if (Total_Instructions > 0) icache_hit_rate = (Total_Instructions - icache_misses) / Total_Instructions;
		  //if (dcache_hits > 0) dcache_hit_rate = (dcache_hits - dcache_misses) / dcache_hits;
		  //if (Total_Instructions > 0) CPI = (float)cycle_count / (float) Total_Instructions;
