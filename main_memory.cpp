#include "main_memory.hpp"

memory::memory(){
	block_size = 4;
	counter = 0;
	for(int i = 0 ; i < mem_size; i++){
		memz[i] = 0;
	}
	
}

bool memory::shouldDelay(){
	return (counter >= 0);
}


void memory::write(int32_t address, int32_t data){
		memz[address] = data;
}

int32_t memory::read(int address){
	return memz[address];
}

void memory::addtoCounter(int time){
	counter += time;
}
