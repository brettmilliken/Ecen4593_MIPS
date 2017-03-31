#include <iostream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char* argv[]){
	//grab all instructions from input file and place them in the instructions array
	//simulating instruction memory
	ifstream inFile;
	instruction temp;
	int i = 0
	
	inFile.open(argv[0])
	if(inFile.is_open){
		cout << "File opened successfully" << endl;
		while(getline(inFile, line)){
			temp.hex = line;
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
	
	}
}
