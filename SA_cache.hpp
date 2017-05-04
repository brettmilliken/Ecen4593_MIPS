#include <cstdio>  
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <stack>
#include <string>
#include <iostream>
#include <queue>

#ifndef SACACHE_HPP
#define SACACHE_HPP

#define words_per_line 4
#define num_of_sets 32
#define lines_per_set 2
#define I_num_of_sets 64
#define I_lines_per_set 1
#define address_size 32
#define miss_penalty 40

//Global Variables
const int set_bit_size = log2(num_of_sets);
//const int offset_size = log2(bytes_per_line);
const int Tag_Size = address_size - set_bit_size;

struct cache_stats{
	int access_count;
	int hit_count;
	int miss_count;
	int read_count;
	int write_count;
	int write_hit;
	int read_hit;
	double hit_rate;	
} status = {0,0,0,0,0,0,0,0.0};

struct buffer_entry{
	int32_t address;
	int data;
};

class cacheLine{
	public:
		int tag;
		int offset;
		bool valid;
		bool dirty;
		int lru; //mod 4 integer
		int data[words_per_line];
		cacheLine();
		~cacheLine();
};

class set{
	public:
	cacheLine* cachelines[lines_per_set];
	set(int lines);
	~set();
	int importWord(int Tag, int setIndex, int offset);
	void streamIn(int index,int tag, int offset, int newData, bool validity);
	int streamOut(int lineIndex, int offset);
	bool emptyLineAvailable();
	int evict_LRU();
	
	private:
	void updateLRU(int lineIndex);
	void replaceLine(int lineIndex, int32_t address, int32_t data);
	
};

class cache{
	public:
	set* sets[num_of_sets]; 
	std::queue<buffer_entry> buffer; 
	cache(int lines_in_set, int set_num);
	~cache();
	int read(int32_t address, bool doesCount,int &clk_cycle);
	void write(int32_t address, int data, int &clk_cycle);
	void loadBuffer(int32_t address, int data);
	int early_start(int32_t address, int &cycle);
	void writeFromBuffer();
	void clk_buffer();
	
	private:
	int getSetIndex(int32_t address);
	int isHit(int tag, int setIndex);
};
#endif
