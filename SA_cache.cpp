#include "SA_cache.hpp"
#include "main_memory.hpp"
#include "main_memory.cpp"
using namespace std;

#define miss_penalty 14

memory main_memory;
bool _DEBUG = true;
bool early_start = false;
bool write_through = false;

void printStatus(){
	status.access_count = status.read_count + status.write_count;
	status.hit_count = status.read_hit + status.write_hit;
	status.hit_rate = (double) status.hit_count/status.access_count;

	printf("|-------STATISTICS--------|");
	printf("access count: %d (read count: %d write count: %d)\n",status.access_count, status.read_count, status.write_count);
	printf("hit rate: %f \n", status.hit_rate);
	printf("hit count: %d (read hit count: %d  write hit count: %d ) \n", status.hit_count, status.read_hit, status.write_hit);
	printf("miss count: %d \n", status.miss_count);
	printf("|-------------------------|\n");	
}


//CACHELINE CLASS
cacheLine::cacheLine(){
	tag = 0x0;
	valid = false;
	
	dirty = false;
	lru = 0; 
	for(int i = 0; i<words_per_line;i++)
		data[i] = 0;
	}

	
	bool tagCompare(int a, int b){
		return (a == b);
	}
	
	int getTag(int32_t address){
		int tag = (address & 0xFFFFFF80);
		tag = tag >> 7;
		return tag;
	}
	
	int getOffset(int32_t address){
		int offset = (address & 0x3);
		return offset;
	}

//SET CLASS
	set::set(int lines){
		for(int i = 0; i< lines; i++)
			cachelines[i] = new cacheLine();
	}
	
	int set::importWord(int Tag, int setIndex, int offset){
		int address = (Tag << 7) + (setIndex << 2) + offset;
		
		int lineIndex;
		for(int i=0; i < lines_per_set; i++){
			if(_DEBUG) printf("Line: %d Valid Bit: %d \n", i, cachelines[i]->valid);
			if(cachelines[i]->valid == false){
				lineIndex = i;
				break;
			}
		}
		printf("Reading from Memory \n");
		printf("Mem Address: %d", address);
		int data = main_memory.read(address);
		printf(" Data: %d \n",data);
		if(offset < words_per_line-1){
			streamIn(lineIndex,Tag,offset,data,false);
		}
		else{
			streamIn(lineIndex,Tag,offset,data,true);
		}
		return data;
	}
	
	//Inputs new data into a cacheline that is available or evicts the least
	//recently used in orderto place new cache entry
	void set::streamIn(int index,int tag, int offset, int newData, bool validity){
		if(_DEBUG) printf("Stream IN-lineIndex: %d Tag: %d Offset: %d Data: %d \n",index,tag, offset, newData);
		cachelines[index]->data[offset] = newData;
		cachelines[index]->tag = tag;
		if(validity){
			cachelines[index]->valid = true;
			updateLRU(index);
		}
		else{
			cachelines[index]->valid = false;
			cachelines[index]->dirty = false;
		}	
		return;
	}
	
	//Streams out data from a cacheline
	int set::streamOut(int lineIndex, int offset){
		updateLRU(lineIndex);
		return cachelines[lineIndex]->data[offset];
	}
	
	//Updates least recently used line in a set
	void set::updateLRU(int lineIndex){
		for(int i=0; i< lineIndex;i++){
			if(cachelines[i]->valid){
				cachelines[i]->lru++;
			}
		}
		cachelines[lineIndex]->lru = 0;
		return;
	}
	
	//replace cache entry at 'lineIndex' with new cache entry 'address'
	
	
	int set::evict_LRU(){
		for(int i= 0; i<lines_per_set; i++){
			if(cachelines[i]->lru == (lines_per_set-1)){
				cachelines[i]->valid = false;
				return i;
			}
		}
		return -1;
	}
	
	bool set::emptyLineAvailable(){
		for(int i=0; i<lines_per_set;i++){
			if(cachelines[i]->valid == false){
				return true;
			}
		}
		return false;
	}
	


//D CACHE CLASS
	cache::cache(int lines_in_set, int set_num){
		for(int i=0;i < set_num; i++)
			sets[i] = new set(lines_in_set);
	}
	
	int cache::getSetIndex(int32_t address){
		int index = (address & 0x7C);
		index = index >> 2;
		return index;
	}
	
	int cache::isHit(int tag, int setIndex){
		for(int i = 0; i < lines_per_set;i++){
			if(sets[setIndex]->cachelines[i]->tag == tag){
				printf("IS HIT: TAGS ARE THE SAME - Line: %d \n",i);
				if(sets[setIndex]->cachelines[i]->valid){
					printf("IS HIT: VALID - Returning line: %d \n",i);
					return i;
				}
			}
		}
		printf("NO HITS \n");
		return -1;
	}
	
	int cache::read(int32_t address, bool doesCount,int &clk_cycle){
		int setIndex = getSetIndex(address);
		int offset = getOffset(address);
		int tag = getTag(address);
		int data[words_per_line];
		int net_cycle = 0;
		
		if(doesCount) status.read_count++;
		int hit = isHit(tag, setIndex);
		printf("Tag: %d Set Index: %d Offset: %d Hit: %d \n",tag,setIndex,offset, hit);
		if(hit >= 0){	
			if(_DEBUG) printf("Read: Hit \n");
			if(doesCount) status.read_hit++;
			return sets[setIndex]->streamOut(hit, offset);
		}
		else {
			if(_DEBUG) printf("Read: Miss \n");
			if(doesCount) status.miss_count++;
			if(!sets[setIndex]->emptyLineAvailable()){
				if(_DEBUG) printf("Evicting... \n");
				int lru = sets[setIndex]->evict_LRU();
				if(!write_through){
					if(_DEBUG) printf("Writing Back to the MEMZ \n");
					int evict_data;
					int evict_tag = sets[setIndex]->cachelines[lru]->tag;
					int evict_address = (evict_tag << 7) + (setIndex << 2);
					for(int evict_off = 0; evict_off < words_per_line; evict_off++){
						evict_data = sets[setIndex]->streamOut(lru,evict_off);
						main_memory.write(evict_address+evict_off, evict_data);
					}
					main_memory.counter += 14;
				}
			}
			if(_DEBUG) printf("Read Miss: Line Available \n");
			int i = 0;
			main_memory.counter += 7;
			while( i < words_per_line) {
				if(main_memory.counter == 0){
					if(_DEBUG) printf("TRUE-OFFSET: %d \n",i);
					main_memory.counter += 1;
					data[i] = sets[setIndex]->importWord(tag, setIndex, i);
					net_cycle++;
					i++;
				}	
				else {
					if(_DEBUG) printf("FALSE-COUNTER: %d \n", main_memory.counter);
					main_memory.counter--;
					net_cycle++;
				}
			}
			if(_DEBUG) printf("NET CYCLE: %d", net_cycle);
			clk_cycle += (net_cycle - 3 + offset);
			printf("CACHE-FILL COMPLETE \n");
			return data[offset];
		}
	}

	
		








/*
	int cache::early_start(int32_t address, int &cycle){
		if(_DEBUG) printf("CACHE_FILL \n");
		int net_cycle = 0;
		int tag = getTag(address);
		int add_offset = getOffset(address);
		int setIndex = getSetIndex(address);
	//	int block = address - add_offset;
		int offset = 0;
		while(offset < words_per_line){
			if(main_memory.counter == 0){
				if(offset == 0){
					if(_DEBUG) printf("TRUE-Initial \n");
					main_memory.counter += 7;
					offset++;
					return true;
				}
				else 
				{
					if(_DEBUG) printf("TRUE-OFFSET: %d \n",offset);
					main_memory.counter = 1;
					if(!sets[setIndex]->emptyLineAvailable()){
						if(_DEBUG) printf("CACHE-FILL-Eviction \n");
						sets[setIndex]->evict_LRU();
					}
						sets[setIndex]->importWord(tag, setIndex, offset);
						offset++;
						if(offset == words_per_line-1){
							main_memory.counter += 1;
							sets[setIndex]->importWord(tag, setIndex, offset);
							net_cycle++;
							printf("CACHE-FILL COMPLETE \n");
							return true;
						}
						main_memory.counter += 1;
						net_cycle++;
						return true;
				}
			}
			else {
				if(_DEBUG) printf("FALSE-COUNTER: %d \n", main_memory.counter);
				main_memory.counter--;
				net_cycle++;
				return false;
			}
		}
		return 	
	}
*/
	
	//Write Policy: 
	void cache::write(int32_t address, int32_t data){
		int tag = getTag(address);
		int setIndex = getSetIndex(address);
		int offset = getOffset(address);
		int block = address-offset;
		status.write_count++;
		int hit = isHit(tag, setIndex);
		if(hit >= 0){
			if(_DEBUG) printf("Write: HIT! \n");
			status.write_hit++  ;
			sets[setIndex]->streamIn(hit,tag,offset,data,true);
			if(write_through){			
				loadBuffer(address, data);
			}
			else {
				sets[setIndex]->cachelines[hit]->dirty = true;
			}
			return;
		}
		else{ 
			if(_DEBUG) printf("Write:MISS \n");
			status.miss_count++;
			int newData;
			if(!sets[setIndex]->emptyLineAvailable()){
				if(_DEBUG) printf("Evicting... \n");
				int lru = sets[setIndex]->evict_LRU();
				if(!write_through && sets[setIndex]->cachelines[lru]->dirty){
					if(_DEBUG) printf("Writing Back to the MEMZ \n");
					int evict_data;
					int evict_tag = sets[setIndex]->cachelines[lru]->tag;
					int evict_address = (evict_tag << 7) + (setIndex << 2);
					for(int evict_off = 0; evict_off < words_per_line; evict_off++){
						evict_data = sets[setIndex]->streamOut(lru,evict_off);
						main_memory.write(evict_address+evict_off, evict_data);
						if(_DEBUG) printf("Memory Write \n Adress - %d Data - %d \n",evict_address, evict_data);
					}
					main_memory.counter += 14;
				}
			}
			if(_DEBUG) printf("Reading in Block... \n");
			int lineIndex;
			for(int i=0; i < lines_per_set; i++){
				if(sets[setIndex]->cachelines[i]->valid == false){
					lineIndex = i;
					break;
				}
			}
			for(int j = 0; j<words_per_line-1;j++){
				newData = main_memory.read(block+j);
				sets[setIndex]->streamIn(lineIndex, tag, j,newData,false); 
			}
			newData = main_memory.read(block+words_per_line);
			sets[setIndex]->streamIn(lineIndex, tag, words_per_line-1,newData,true);
			sets[setIndex]->streamIn(lineIndex, tag, offset,data,true); 
			loadBuffer(address,data);
			return;
		}
	}
	
	void cache::loadBuffer(int32_t address, int32_t data){
		buffer_entry entry;
		int32_t temp1, temp2;
		entry.address = address;
		entry.data = data;
		buffer.push(entry);
		entry = buffer.front();
		temp1 = entry.data;
		temp2 = entry.address;
		if(_DEBUG) printf("Loading Buffer... \n Buffer Front: DATA -  %d ADDRESS - %d \n", temp1, temp2);
		
	}
	
	void cache::clk_buffer(){
		if(main_memory.counter == 0){
			writeFromBuffer();
		}
		else {
			main_memory.counter--;
		}
		return;
	}
	
	void cache::writeFromBuffer(){
		buffer_entry temp = buffer.front();
		if(_DEBUG) printf("Writing to Memory \n");
		buffer.pop();
		if(_DEBUG) printf("Memory Write \n Adress - %d Data - %d \n",temp.address, temp.data);
		main_memory.write(temp.address, temp.data);
		main_memory.counter = miss_penalty;
	}
	
/*

int main(){
	int clk = 0;
	int d;
	for(int a = 551; a<650; a++){
		d = rand();
		main_memory.write(a, d);
	}
	printf("Reading From Memory \n");
	for(int i = 551; i<650; i++){
		d = main_memory.read(i);
		cout << "ADDRESS:" << i << " DATA: " << d << endl;
	}
	
	
	cache* icache = new cache(lines_per_set,num_of_sets);
		main_memory.counter = 0;
		int data = 0;
	//	int cache_read;
	
		printf("BEGINNING READ TEST \n");
		for(int i = 560; i < 580; i+=2){
				printf("READING: Address-%d \n",i);
				data = icache->read(i,true,clk);
				printf("DATA: %d \n",data);
			}
			
					printf("\n FINAL CLOCK: %d \n",clk); 

			
		printf("BEGINNING WRITE TEST \n");
		for(int i = 580; i < 600; i+=2){
			data = rand() % 100;
			printf("WRITING: Address-%d DATA-%d \n",i,data);
			icache->write(i,data);
			data = icache->read(i,true, clk);
			icache->clk_buffer();
			printf("READ DATA: %d \n",data);
			clk++;
		}
		printf("FINAL CLOCK: %d \n",clk); 
		
		//cache* icache = new cache(I_lines_per_set,num_of_sets);
	
	
	return 1;
}
*/
