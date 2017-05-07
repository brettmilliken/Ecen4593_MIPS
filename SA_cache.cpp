#include "SA_cache.hpp"
#include "main_memory.hpp"
using namespace std;
//NEw
//#define miss_penalty 14

memory main_memory;
bool _DEBUG = false;
bool _DEBUG1 = false;
bool _DEBUG2 = false;
bool _DEBUG3 = false;
bool _DEBUGADDRESS = false;
bool _EVICT = false;
bool stats = true; // toggles basic statistics for print status
bool wordstats = true; // toggles word import stats
bool hitstats = false; // toggles hit statistic printing
bool fillprint = false; // toggles cache fill complete print
bool taginfo = false; // toggles tag information on tag access

int indexBits = log2(num_of_sets);
int offsetBits = log2(words_per_line);
int tagBits = 32 - indexBits - offsetBits;
extern cache_stats cachestatus;

void printStatus(){
	cachestatus.access_count = cachestatus.read_count + cachestatus.write_count;
	cachestatus.hit_count = cachestatus.read_hit + cachestatus.write_hit;
	cachestatus.hit_rate = (double) cachestatus.hit_count/cachestatus.access_count;
	if(stats){
		printf("|-------STATISTICS--------|");
		printf("access count: %d (read count: %d write count: %d)\n",cachestatus.access_count, cachestatus.read_count, cachestatus.write_count);
		printf("hit rate: %f \n", cachestatus.hit_rate);
		printf("hit count: %d (read hit count: %d  write hit count: %d ) \n", cachestatus.hit_count, cachestatus.read_hit, cachestatus.write_hit);
		printf("miss count: %d \n", cachestatus.miss_count);
		printf("|-------------------------|\n");
	}	
}


//CACHELINE CLASS
cacheLine::cacheLine(){
	tag = 0;
	valid = false;
	
	dirty = false;
	lru = 0; 
	for(int i = 0; i< words_per_line;i++)
		data[i] = 0;
	}

	
	bool tagCompare(int a, int b){
		return (a == b);
	}
	
	int getTag(int32_t address){
		switch (offsetBits+indexBits){
			case 0 : return address; break;
			case 1 : return (address & ~0x00000001) >> (offsetBits+indexBits); break;
			case 2 : return (address & ~0x00000003) >> (offsetBits+indexBits); break;
			case 3 : return (address & ~0x00000007) >> (offsetBits+indexBits); break;
			case 4 : return (address & ~0x0000000F) >> (offsetBits+indexBits); break;
			case 5 : return (address & ~0x0000001F) >> (offsetBits+indexBits); break;
			case 6 : return (address & ~0x0000003F) >> (offsetBits+indexBits); break;
			case 7 : return (address & ~0x0000007F) >> (offsetBits+indexBits); break;
			case 8 : return (address & ~0x000000FF) >> (offsetBits+indexBits); break;
			case 9 : return (address & ~0x000001FF) >> (offsetBits+indexBits); break;
			case 10 : return (address & ~0x000003FF) >> (offsetBits+indexBits); break;
			default: return address; break;
		}
	}
	
	int getOffset(int32_t address){
		switch (offsetBits){
			case 0 : return 0;
			case 1 : return (address & 0x1);
			case 2 : return (address & 0x3);
			case 3 : return (address & 0x7);
			case 4 : return (address & 0xF);
			default: return address;
		}
	}

//SET CLASS
	set::set(int lines){
		int i;
		//cacheLine* temp;
		//for(i = 0; i< lines; i++)
		//	temp = new cacheLine();
		//	cachelines[i] = temp;
		cachelines = (cacheLine*) malloc((sizeof(cacheLine) * lines));
		for(i = 0; i< lines; i++) {
			 new (cachelines + i) cacheLine();
		}
	}
	
	int set::importWord(int Tag, int setIndex, int offset){
		int address = (Tag << (indexBits+offsetBits)) + (setIndex << (offsetBits)) + offset;
		
		int lineIndex;
		for(int i=0; i < lines_per_set; i++){
			if(_DEBUG) printf("Line: %d Valid Bit: %d \n", i, cachelines[i].valid);
			if(cachelines[i].valid == false){
				lineIndex = i;
				break;
			}
		}
		if(wordstats){
			printf("Reading from Memory \n");
			printf("Mem Address: %d", address);
		}
		int data = main_memory.read(address);
		if(wordstats){	
			printf(" Data: %d \n",data);
		}
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
		//printf("Stream IN-lineIndex: %d Tag: %d Offset: %d Data: %d \n",index,tag, offset, newData);
		cachelines[index].data[offset] = newData;
		cachelines[index].tag = tag;
		if(validity){
			cachelines[index].valid = true;
			updateLRU(index);
		}
		else{
			cachelines[index].valid = false;
			cachelines[index].dirty = false;
		}	
		return;
	}
	
	//Streams out data from a cacheline
	int set::streamOut(int lineIndex, int offset){
		updateLRU(lineIndex);
		return cachelines[lineIndex].data[offset];
	}
	
	//Updates least recently used line in a set
	void set::updateLRU(int lineIndex){
	//	if(_EVICT) printf("Updating LRU \n");
		for(int i=0; i< lines_per_set;i++){
		//	if(_EVICT) printf("Line %d LRU: %d \n",i,cachelines[i].lru);
			if(cachelines[i].valid){
				if(i == lineIndex){
					cachelines[i].lru = 0;
			//		if(_EVICT) printf("Line %d LRU: %d \n",i,cachelines[i].lru);
				}
				else {
					cachelines[i].lru = 1;
				}
			}
		}
		cachelines[lineIndex].lru = 0;
		return;
	}
	
	//replace cache entry at 'lineIndex' with new cache entry 'address'
	
	
	int set::evict_LRU(){
		for(int i= 0; i<lines_per_set; i++){
			if(cachelines[i].lru == (lines_per_set-1)){
				cachelines[i].valid = false;
				if(_EVICT) printf("Eviction: Line %d LRU: %d \n",i,cachelines[i].lru);
				return i;
			}
		}
		return -1;
	}
	
	bool set::emptyLineAvailable(){
		for(int i=0; i<lines_per_set;i++){
			if(cachelines[i].valid == false){
				return true;
			}
		}
		return false;
	}
	
	set::~set(){
		free(cachelines);
	}
	


//D CACHE CLASS
	cache::cache(int lines_in_set, int set_num){
		set_number = set_num;
		line_number = lines_per_set;
		for(int i=0;i < set_num; i++)
			sets[i] = new set(lines_in_set);
	}
	
	int cache::getSetIndex(int32_t address){
		int extracted_bits = 0;
		int block_size = offsetBits+indexBits;
		switch(block_size){
			case 0 :  extracted_bits = 0; break;
			case 1 :  extracted_bits = (address & 0x00000001); break; 
			case 2 :  extracted_bits = (address & 0x00000003); break;
			case 3 :  extracted_bits = (address & 0x00000007); break;
			case 4 :  extracted_bits = (address & 0x0000000F); break;
			case 5 :  extracted_bits = (address & 0x0000001F); break;
			case 6 :  extracted_bits = (address & 0x0000003F); break;
			case 7 :  extracted_bits = (address & 0x0000007F); break;
			case 8 :  extracted_bits = (address & 0x0000001F); break;
			case 9 :  extracted_bits = (address & 0x000001FF); break;
			default: extracted_bits = 0; break;
		}
		if(_DEBUGADDRESS) printf("SET BLOCK: %d BITS: %d \n",extracted_bits,block_size);
		//uint temp = extracted_bits >> offsetBits;
		//printf("Extracted INDEX: %d",temp);
		return (extracted_bits >> offsetBits);
	}
	
	int cache::isHit(int tag, int setIndex){
		for(int i = 0; i < lines_per_set;i++){
			if(sets[setIndex]->cachelines[i].tag == tag){
				//if(hitstats) printf("IS HIT: TAGS ARE THE SAME - Line: %d \n",i);
				if(sets[setIndex]->cachelines[i].valid){
					//if(hitstats) printf("IS HIT: VALID - Returning line: %d \n",i);
					return i;
				}
			}
		}
		//printf("NO HITS \n");
		return -1;
	}
	
	int cache::read(int32_t address, bool doesCount,int &clk_cycle){
		if(_DEBUG1) printf("READ ADDRESS: %d \n", address);
		int setIndex = getSetIndex(address);
		int offset = getOffset(address);
		int tag = getTag(address);
		int data[words_per_line];
		int net_cycle = 0;
		if(_DEBUGADDRESS) {
			printf("ADDRESS: %d \n",address);
			printf("TAG: %d ",tag);
			printf("SET INDEX: %d ", setIndex);
			printf("OFFSET: %d \n", offset);
		}
		if(doesCount) cachestatus.read_count++;
		int hit = isHit(tag, setIndex);
		if(taginfo) printf("Tag: %d Set Index: %d Offset: %d Hit: %d \n",tag,setIndex,offset, hit);
		if(hit >= 0){	
			//if(_DEBUG) printf("Read: Hit \n");
			//printf("Offset: %d",offset);
			if(doesCount) cachestatus.read_hit++;
			return sets[setIndex]->streamOut(hit, offset);
		}
		else {
			if(_DEBUG) printf("Read: Miss \n");
			if(doesCount) cachestatus.miss_count++;
			if(!sets[setIndex]->emptyLineAvailable()){
				if(_DEBUG) printf("Evicting... \n");
				int lru = sets[setIndex]->evict_LRU();
				if(!write_through){
					if(_DEBUG) printf("Writing Back to the MEMZ \n");
					if(_DEBUG) printf("LRU: %d",lru);
					int evict_data;
					int evict_tag = sets[setIndex]->cachelines[lru].tag;
					if(_DEBUG) printf("evict tag: %d setIndex %d \n",evict_tag, setIndex);
					int evict_address = ((evict_tag << (indexBits + offsetBits)) + (setIndex << offsetBits));
					//printf("newly configured address: %d",evict_address);
					for(int evict_off = 0; evict_off < words_per_line; evict_off++){
						evict_data = sets[setIndex]->streamOut(lru,evict_off);
						main_memory.write(evict_address+evict_off, evict_data);
					}
					net_cycle += 6+(words_per_line-1)*2;
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
			if(_DEBUG) printf("NET CYCLE: %d \n", net_cycle);
			clk_cycle += (net_cycle + offset);
			if(fillprint) printf("CACHE-FILL COMPLETE \n");
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
	void cache::write(int32_t address, int data, int &clk_cycle){
		if(_DEBUG1 | _EVICT) printf("WRITE!! \n \n");
		int tag = getTag(address);
		int setIndex = getSetIndex(address);
		int offset = getOffset(address);
		int block = address-offset;
		if(_DEBUGADDRESS) {
			printf("ADDRESS: %d \n",address);
			printf("TAG: %d ",tag);
			printf("SET INDEX: %d ", setIndex);
			printf("OFFSET: %d \n", offset);
			
		}
		cachestatus.write_count++;
		int hit = isHit(tag, setIndex);
		if(hit >= 0){
			if(_DEBUG1) printf("Write: HIT! \n");
				cachestatus.write_hit++  ;
				sets[setIndex]->streamIn(hit,tag,offset,data,true);
			if(write_through){			
				loadBuffer(address, data);
			}
			else {
				sets[setIndex]->cachelines[hit].dirty = true;
			}
			return;
		}
		else{ 
			if(_DEBUG1) printf("Write:MISS \n");
			cachestatus.miss_count++;
			int newData;
			if(!sets[setIndex]->emptyLineAvailable()){
				if(_EVICT) printf("Evicting... \n");
				int lru = sets[setIndex]->evict_LRU();
				if(!write_through && sets[setIndex]->cachelines[lru].dirty){
					if(_EVICT) printf("Eviction: Write Back: Writing to Memory \n");
					int evict_data;
					int evict_tag = sets[setIndex]->cachelines[lru].tag;
					int evict_address = ((evict_tag << (indexBits + offsetBits)) + (setIndex << offsetBits));
					for(int evict_off = 0; evict_off < words_per_line; evict_off++){
						evict_data = sets[setIndex]->streamOut(lru,evict_off);
						main_memory.write(evict_address+evict_off, evict_data);
						if(_EVICT) printf("Eviction: Memory Write \n Address - %d Data - %d \n",evict_address, evict_data);
					}
					main_memory.counter += (6+(words_per_line-1)*2);
				}
			}
			if(_DEBUG1) printf("Reading in Memory Block \n");
			int lineIndex;
			for(int i=0; i < lines_per_set; i++){
				if(sets[setIndex]->cachelines[i].valid == false){
					lineIndex = i;
					break;
				}
			}
			for(int j = 0; j<words_per_line-1;j++){
				newData = main_memory.read(block+j);
				sets[setIndex]->streamIn(lineIndex, tag, j,newData,false); 
				if(_DEBUG2) printf("NEW DATA: %d ADDRESS: %d \n",newData, (block+j));
			}
			newData = main_memory.read(block+words_per_line);
			sets[setIndex]->streamIn(lineIndex, tag, words_per_line-1,newData,true);
			sets[setIndex]->streamIn(lineIndex, tag, offset,data,true); 
			if(!write_through){
				sets[setIndex]->cachelines[lineIndex].dirty = true;
			}
			else{
				loadBuffer(address,data);
			}
			if(_DEBUG2) printf("WRITE OUTPUT: NEW DATA: %d ADDRESS: %d \n",newData, address);
			clk_cycle += ((8+(2*words_per_line) + main_memory.counter));
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

