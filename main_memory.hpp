#include<cstdio>  
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<stack>
#include<string>
#include<iostream>

#ifndef MM_INC	
#define MM_INC
#define mem_size 10000

class memory {
	public:
	memory();

	int32_t memz[mem_size];
	int block_size;
	int counter;
	bool busy;
	
	int32_t read(int32_t address);
	void write(int32_t address, int32_t data);		
	bool shouldDelay();
	void addtoCounter(int time);
	
};





#endif
