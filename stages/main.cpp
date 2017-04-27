#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include "instructions.hpp" 
#include "pc.hpp" 
#include "defstage.cpp" 
using namespace std;
// define global variables
instr instructions[1000]; // define instruction memory
pc PC; // define program counter
IF_ID ifid; // if/id stage register
IF_ID shadifid;
ID_EX idex; // id/ex stage register
ID_EX shadidex;
EX_MEM exmem; // ex/mem stage register
EX_MEM shadexmem;
MEM_WB memwb; // mem/wb stage register
MEM_WB shadmemwb;
int gregisters[32];
int memory[1200];
// end define global variables

int main(int argc, char* argv[]){
	//grab all instructions from input file and place them in the instructions array
	//simulating instruction memory
	ifstream inFile;
	instr temp;
	int i = 0;	
	inFile.open(argv[0]);
	if(inFile.is_open){
		std::string line;
		cout << "File opened successfully" << endl;
		while(getline(inFile, line)){
			temp.value = line;
			instructions[i] = temp;
			i++;
			if(i > 999){
				cout << "ERROR: Instruction Memory is full" << endl;
				break;
			}
		}
	}
	else{
	cout << "Instruction File couldn't be opened" << endl;
	}
	
	while(1){
		
		//actual pipelining code
		IF();
		ID();
		EX();
		MEM();
		WB();
		shadToNorm();
	
	}
}
