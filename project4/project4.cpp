#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <string>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdlib.h>
#include <cmath>

using namespace std;

class L1_cache; class L2_cache;

static int L2_capacity=0, L1_capacity=0;
static int L2_as=0, L1_as=0;
static int block_size=0;

static int L1_access_time_w=0,L1_access_time_r=0,L2_access_time_w=0, L2_access_time_r=0;
static int lru_num=0, access_count=0, r_count=0, w_count=0;
static int L1_miss_r=0, L1_miss_w=0;
static int L2_miss_r=0, L2_miss_w=0;
static int L1_evict_clean=0, L1_evict_dirty=0,L2_evict_clean=0, L2_evict_dirty=0;

int input_option_a = 0;
int input_option_b = 0;
int input_option_c = 0;
int input_option_evict = 0;

int block_offset_size = 0;
int index_bit_size =0;
int tag_bit_size =0;

L1_cache** L1;
L2_cache** L2;


int cache_hit_check(string level,L1_cache l1_new, L2_cache l2_new);
void L1_cache_operator(string str);
void L2_cache_operator(string str);

string hex_to_bi(string hexnum){
    stringstream stream;
    stream << std::hex << hexnum;
    unsigned long long n;
    stream >> n;
    bitset<64> bit(n);
    return bit.to_string();
}

class L1_cache {
    public:
        int lru=0;
        int vaild=0;
        int dirty_bit=0;
        long long int address=0;
        long long int index=0;
        long long int tag=0;
        L1_cache(){};
        L1_cache(long long int add, long long int ind, long long int tg){address=add; index=ind; tag=tg;};
};

class L2_cache {
    public:
        int lru=0;
        int vaild=0;
        int dirty_bit=0;
        long long int address=0;
        long long int index=0;
        long long int tag=0;
        L2_cache(){};
        L2_cache(long long int add, long long int ind, long long int tg){address=add; index=ind; tag=tg;};
};

void L1_cache_operator(string str){
    access_count++;
    string type = str.substr(0,1);
    string add_str = str.substr(2);
    string add_bit = hex_to_bi(add_str);
    long long int add = stoull(add_str,nullptr,16);
    long long int tag = stoi(add_bit.substr(0,tag_bit_size),nullptr,2);
    long long int ind = stoi(add_bit.substr(tag_bit_size,index_bit_size),nullptr,2);
    L1_cache l1_new(add,ind,tag);
    L2_cache l2_new(add,ind,tag);
    if (type == "w" | type == "W") { L1_access_time_w++;}
    else if (type == "R" | type == "r") {L1_access_time_r++;}
    
    if (cache_hit_check("L1",l1_new,l2_new)==1){
        if (type == "w" | type == "W") { }
        else if (type == "R" | type == "r") { }
    } //L1 cache Hit

    else if (cache_hit_check("L1",l1_new,l2_new)==0){
        if (cache_hit_check("L2",l1_new,l2_new)==1){
            if (type == "w" | type == "W") {L2_access_time_w++; }
            else if (type == "R" | type == "r") {L2_access_time_r++; }
            int l1_position=0;
            int find_sign=0;
            for (int i = 0; i < L1_as; i++) {
                if (L1[l1_new.index][i].vaild == 0) {
                l1_position = i;
                find_sign=1;
                break;
                }
            }
            if (find_sign==1){
                l1_new.vaild=1;
                L1[l1_new.index][l1_position] = l1_new;
            }
            else if (find_sign==0 & L1[l1_new.index][l1_position].vaild != 0 &input_option_evict == 1){
                int find_signal;
                for (int i = 1; i < L1_as; i++) {
                    if (L1[l1_new.index][i].lru < L1[l1_new.index][i-1].lru) {
                        l1_position = i;
                        find_signal=1;
                        break;
                        }
                }
            if (type == "w" | type == "W") {l1_new.dirty_bit=1;}
            if(find_signal==0){l1_position = 0;}
            l1_new.vaild=1;
            l1_new.lru = L1[l1_new.index][l1_position].lru+1;
            if(L1[l1_new.index][l1_position].dirty_bit==1){L1_evict_dirty++;}
            else if(L1[l1_new.index][l1_position].dirty_bit==0){L1_evict_clean++;}
            L1[l1_new.index][l1_position] = l1_new;
            }
            else if (find_sign==0 & L1[l1_new.index][l1_position].vaild != 0 &input_option_evict == 2){
                int random_position = (rand() % (L1_as-1));
                if (type == "w" | type == "W") {l1_new.dirty_bit=1;}
                l1_new.vaild=1;
                l1_new.lru = L1[l1_new.index][random_position].lru+1;
                if(L1[l1_new.index][random_position].dirty_bit==1){L1_evict_dirty++;}
                else if(L1[l1_new.index][random_position].dirty_bit==0){L1_evict_clean++;}
                L1[l1_new.index][l1_position] = l1_new;
            }
        } // L1 miss & L2 hit

        else if (cache_hit_check("L2",l1_new,l2_new)==0){
            L2_cache_operator(str);
        } //L1 miss & L2 miss
    
    }

}

void L2_cache_operator(string str){
    string type = str.substr(0,1);
    string add_str = str.substr(2);
    string add_bit = hex_to_bi(add_str);
    long long int add = stoull(add_str,nullptr,16);
    long long int tag = stoi(add_bit.substr(0,tag_bit_size),nullptr,2);
    long long int ind = stoi(add_bit.substr(tag_bit_size,index_bit_size),nullptr,2);
    L1_cache l1_new(add,ind,tag);
    L2_cache l2_new(add,ind,tag);
    int l1_position=0;
    int l2_position=0;

    if (type == "w" | type == "W") {L2_access_time_w++;L1_miss_w ++;L2_miss_w ++;l1_new.dirty_bit=1;l2_new.dirty_bit=1;}
    else if (type == "R" | type == "r") {L2_access_time_r++;L1_miss_r ++;L2_miss_r ++;}
    int find_sign=0;
    for (int i = 0; i < L1_as; i++) {
        if (L1[l1_new.index][i].vaild == 0) {
            l1_position = i;
            find_sign=1;
            break;
            }
        }
    if (l1_position < L1_as & L1[l1_new.index][l1_position].vaild == 0){
        l1_new.vaild=1;
        L1[l1_new.index][l1_position] = l1_new;
        //if(l1_new.dirty_bit==1){L1_evict_dirty++;}
        //else if(l1_new.dirty_bit==0){L1_evict_clean++;}
    }
    else if (find_sign==0&l1_position == L1_as & L1[l1_new.index][l1_position].vaild != 0&input_option_evict==1){
        int find_signal;
        for (int i = 1; i < L1_as; i++) {
            if (L1[l1_new.index][i].lru < L1[l1_new.index][i-1].lru) {
                l1_position = i;
                find_signal=1;
                break;
                }
        }
        if(find_signal==0){l1_position = 0;}
        l1_new.vaild=1;
        l1_new.lru = L1[l1_new.index][l1_position].lru+1;
        L1[l1_new.index][l1_position] = l1_new;
        if(L1[l1_new.index][l1_position].dirty_bit==1){L1_evict_dirty++;}
        else if(L1[l1_new.index][l1_position].dirty_bit==0){L1_evict_clean++;}
    }
    else if (find_sign==0 & L1[l1_new.index][l1_position].vaild != 0 &input_option_evict == 2){
                int random_position = (rand() % (L1_as-1));
                if (type == "w" | type == "W") {l1_new.dirty_bit=1;}
                l1_new.vaild=1;
                l1_new.lru = L1[l1_new.index][random_position].lru+1;
                if(L1[l1_new.index][random_position].dirty_bit==1){L1_evict_dirty++;}
                else if(L1[l1_new.index][random_position].dirty_bit==0){L1_evict_clean++;}
                L1[l1_new.index][l1_position] = l1_new;
            }
    //Level2 cache
    int find_signal=0;
    for (int i = 0; i < L2_as; i++) {
        if (L2[l2_new.index][i].vaild == 0) {
            l2_position = i;
            find_signal=1;
            break;
            }
        }
    if(find_signal==1){
        l2_new.vaild=1;
        L2[l2_new.index][l2_position] = l2_new;
        ////////
    }
    else if (find_signal==0 & L2[l2_new.index][l2_position].vaild != 0){
        int find_signal2;
        if(input_option_evict==1){
        for (int i = 1; i < L2_as; i++) {
            if (L2[l2_new.index][i].lru < L2[l2_new.index][i-1].lru) {
                l2_position = i;
                find_signal2=1;
                break;
                }
        }
        if(find_signal2==0){l2_position = 0;}}
        else if(input_option_evict==2){
            int random_position = (rand() % (L2_as-1));
            l2_position = random_position;
        }
        for (int i = 0; i < L1_as; i++) {
            if (L1[l2_new.index][i].tag == L2[l2_new.index][l2_position].tag) {
                L1[l2_new.index][i] = L1_cache(0,0,0);
                /////////
                break;
            }
        }
        l2_new.vaild=1;
        if(L2[l2_new.index][l2_position].dirty_bit==1){L2_evict_dirty++;}
        else if(L2[l2_new.index][l2_position].dirty_bit==0){L2_evict_clean++;}
        l2_new.lru = L2[l2_new.index][l2_position].lru+1;
        L2[l2_new.index][l2_position] = l2_new;
        
    }


}

int cache_hit_check(string level,L1_cache l1_new, L2_cache l2_new){
    if (level == "L1"){
        for (int i = 0; i < L1_as; i++) {
            if (L1[l1_new.index][i].tag == l1_new.tag) {
                return 1;
            }
        }
        return 0;
    }
    else if (level == "L2"){
        for (int i = 0; i < L2_as; i++) {
            if (L2[l2_new.index][i].tag == l2_new.tag) {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

int main(int argc, char* argv[]){

    std::ifstream file;
    if (argc == 1){return 0;}
	file.open(argv[argc - 1]); 

    string memory_start;
    string memory_end;
    int counts_limit = 0;
    int instruction_counts = 0;

    if (file.is_open()) {

        //input option check//
		for (int i = 0; i < argc; i++) {
			if (!strcmp(argv[i], "-a")) {
                input_option_a = 1;
                string num(argv[i+1]);
				L2_as = stoi(num);
				if (L2_as >= 4){L1_as = L2_as/4;} 
				else {L1_as = L2_as;}}

			if (!strcmp(argv[i], "-b")) {
				string num(argv[i+1]);
				block_size = stoi(num);
				input_option_b = 1;}

            if (!strcmp(argv[i], "-c")) {
				string num(argv[i+1]);
				L2_capacity = stoi(num);
				if (L2_capacity >= 4){L1_capacity = L2_capacity/4;} 
				else {L1_capacity = L2_capacity;}
				input_option_c = 1;}

			if (!strcmp(argv[i], "-lru")) {
				input_option_evict = 1;}

			if (!strcmp(argv[i], "-random")) {
                if (input_option_evict == 1) {cout<<"Input Option Error : Two replacement policy"<<endl; return 0;}
				input_option_evict = 2;}
			}
        
        if (input_option_a + input_option_b + input_option_c != 3) //input option error handling
            {cout<<"Input Option Error : You have to implement -a -b -c options"<<endl; return 0;}
        if (input_option_evict == 0) //input option error handling
            {cout<<"Input Option Error : You have to implement replacement policy options"<<endl; return 0;}

        block_offset_size = log2(block_size);
        index_bit_size = log2((1024*L1_capacity)/(block_size*L1_as));
        tag_bit_size = 64 - block_offset_size - index_bit_size;
        
        int set_size_L1 = (1024*L1_capacity)/(block_size*L1_as);
        int set_size_L2 = (1024*L2_capacity)/(block_size*L2_as);
        L1 = new L1_cache * [set_size_L1];
        L2 = new L2_cache * [set_size_L2]; 

        for (int i; i < set_size_L1; i++){ L1[i] = new L1_cache[L1_as];}
        for (int j; j < set_size_L2; j++){ L2[j] = new L2_cache[L2_as];}
        
        while (!file.eof()){
            string str_;
            getline(file, str_);
            if(str_.length() < 2)break;
            L1_cache_operator(str_);
		}
        file.close();
    }

    string file_name = argv[argc-1];
	file_name.erase(file_name.find("."));
	file_name.append("_");
	file_name.append(to_string(L2_capacity));
	file_name.append("_");
	file_name.append(to_string(L2_as));
	file_name.append("_");
	file_name.append(to_string(block_size)); 
	file_name.append(".out");

    std::ofstream fout(file_name);

    fout<< "-- General Stats --" << endl;
	fout<< "L1 capacity:  " << L1_capacity << endl;
	fout<< "L1 way:  " << L1_as << endl;
	fout<< "L2 capacity:  " << L2_capacity << endl;
	fout<< "L2 way:  " << L2_as << endl;
	fout<< "Block Size:      " << block_size << endl;
	fout<< "Total accesses:  " << access_count << endl;
	fout<< "Read accesses:  " << L1_access_time_r << endl;
	fout<< "Write accesses:  " << L2_access_time_w << endl;
	fout<< "L1 Read misses:  " << L1_miss_r << endl;
	fout<< "L2 Read misses:  " << L2_miss_r << endl;
	fout<< "L1 Write misses:  " << L1_miss_w << endl;
	fout<< "L2 Write misses:  " << L2_miss_w << endl;
	fout<< "L1 Read miss rate:  " << (float)L1_miss_r/(float)L1_access_time_r*100<<"%"<<endl;
	fout<< "L2 Read miss rate:  " << (float)L2_miss_r/(float)L2_access_time_r*100 << "%" << endl;
	fout<< "L1 Write miss rate:  " << (float)L1_miss_w/(float)L1_access_time_w*100<< "%" << endl;
	fout<< "L2 Write miss rate:  " << (float)L2_miss_w/(float)L2_access_time_w*100 << "%" << endl;
	fout<< "L1 Clean eviction:  " << L1_evict_clean << endl;
	fout<< "L2 Clean eviction:  " << L2_evict_clean << endl;
	fout<< "L1 Dirty eviction:  " << L1_evict_dirty << endl;
	fout<< "L2 Dirty eviction:  " << L2_evict_dirty << endl;
    fout.close();
    cout<<"Process is done"<<endl;

    return 0;
}