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

static int data_size = 0;
static int text_size = 0;
static int position = 0;
static vector<int> Reg;
static vector<string> text_codes, data_codes;
int PC = 0x400000;
int data_address = 0x10000000;
int text_address = 0x400000;

static int IF_ID_Instr=0, IF_ID_NPC =0;
static int ID_EX_NPC=0, ID_EX_rs=0, ID_EX_rt=0, ID_EX_Imm=0, ID_EX_rd=0;
static int EX_MEM_Alu_out=0;
static int EX_MEM_Br_target=0;
static int MEM_WB_Alu_out=0;
static int MEM_WB_Mem_out=0;

static int stall=0, IF_stall=0, ID_stall=0, EX_stall=0, MEM_stall=0, WB_stall=0;
static int IF_end=0, ID_end=0, EX_end=0, MEM_end=0, WB_end=0;
static int if_inst=0,id_inst=0,ex_inst=0,mem_inst=0,wb_inst=0;

static int cycle=0;
static int line_number;

int input_option_m = 0;
int input_option_d = 0;
int input_option_n = 0;
int input_option_p = 0;
int branch_option = 0;
static int branch_signal=0;

int complement2_16bit(string bits){
    int decimal= stoi(bits.substr(1,15),nullptr,2)-(stoi(bits.substr(0,1))*pow(2,15));
    return decimal;
}

string hex_to_bi(string hexnum){
    stringstream stream;
    stream << std::hex << hexnum;
    unsigned n;
    stream >> n;
    bitset<32> bit(n);
    return bit.to_string();
}

string detect_oper(string input_str){
    string s = input_str.substr(0,6);
    if (s == "000000"){ //rtype
        //cout<<"input is" << input_str<<endl;
        s = input_str.substr(input_str.length()-6,6);
    }
    return s;
}

void r_type_oper(string str){
    string oper = detect_oper(str);
    string rs = str.substr(6,5);
    string rt = str.substr(11,5);
    string rd = str.substr(16,5);
    string shamt = str.substr(21,5);
    int rs_int = stoi(rs, nullptr, 2);
    int rt_int = stoi(rt, nullptr, 2);
    int rd_int = stoi(rd, nullptr, 2);
    //cout<<"r_type rt int is "<<rt_int<<endl;
    if (str.substr(0,6) != "000000"){
        return;
    }
    if (oper=="100001"){
        int result_sum = Reg[rs_int]+Reg[rt_int];
        Reg[rd_int] = result_sum;
    } //addu
    else if (oper=="100100"){ 
        int result_and = Reg[rs_int] & Reg[rt_int];
        Reg[rd_int] = result_and;
    } //and
    else if (oper=="001000"){
        //int addr = Reg[rs_int];
        //position = addr-text_address-1;
    } //jr
    else if (oper=="100111"){
        int result_nor = Reg[rs_int] | Reg[rt_int];
        Reg[rd_int] = ~result_nor;
    } //nor
    else if (oper=="100101"){
        int result_or = Reg[rs_int] | Reg[rt_int];
        Reg[rd_int] = result_or;
    } //or
    else if (oper=="101011"){
        int result_sltu=0;
        if (Reg[rs_int]<Reg[rt_int]){ result_sltu=1;}
        Reg[rd_int] = result_sltu;
    } //sltu
    else if (oper=="000000"){
        int shamt_int = stoi(shamt, nullptr, 2);
        int result_sll = Reg[rt_int]*pow(2,shamt_int);
        Reg[rd_int] = result_sll;
    } //sll
    else if (oper=="000010"){
        int shamt_int = stoi(shamt, nullptr, 2);
        int result_slr = Reg[rt_int]/pow(2,shamt_int);
        Reg[rd_int] = result_slr;
    } //srl
    else if (oper=="100011"){
        int result_sum = Reg[rs_int]-Reg[rt_int];
        Reg[rd_int] = result_sum;
    } //subu
    return;
}

void i_type_oper(string str){
    string oper = detect_oper(str);
    string rs = str.substr(6,5);
    string rt = str.substr(11,5);
    string offset = str.substr(16,16);
    int rs_int = stoi(rs, nullptr, 2);
    int rt_int = stoi(rt, nullptr, 2);
    int offset_int = stoi(offset, nullptr, 2);
    //cout<<"Itype rs int is "<<rs_int<<endl;
    if (str.substr(0,6)== "000000"){
        return;
    } // r-type?
    if (oper=="001001"){
        offset_int = complement2_16bit(offset);
        int result_sum = Reg[rs_int]+offset_int;
        Reg[rt_int] = result_sum;
    } //addi
    else if (oper=="001100"){
        int result_and = Reg[rs_int] & offset_int;
        Reg[rt_int] = result_and;
    } //andi
    else if (oper=="001111"){
        int result_lui = offset_int*65536;
        Reg[rt_int] = result_lui;
    } //lui
    else if (oper=="100011"){
        string str = data_codes[(Reg[rs_int]-data_address)/4+offset_int/4];
        string st = str.substr(2);
        int result_lw = stol(st,nullptr,16);
        Reg[rt_int] = result_lw;
    } //lw
    else if (oper=="100000"){
        int rest = offset_int%4;
        int result_lb = 0;
        string str = data_codes[(Reg[rs_int]-data_address)/4+offset_int/4];
        while(str.length()<10){str.insert(2,"0");}
        if (rest==0){result_lb = stoi(str.substr(2,2),nullptr,16);}
        else if (rest ==1){result_lb = stoi(str.substr(4,2),nullptr,16);}
        else if (rest ==2){result_lb = stoi(str.substr(6,2),nullptr,16);}
        else if (rest ==3){result_lb = stoi(str.substr(8,2),nullptr,16);}
        while(str[2] ==0){str.erase(2,1);}
        Reg[rt_int] = result_lb;
    } //lb
    else if (oper=="001101"){
        int result_ori = Reg[rs_int] | offset_int;
        Reg[rt_int] = result_ori;
    } //ori
    else if (oper=="001011"){
        int result_sltiu=0;
        if (Reg[rs_int]<offset_int){ result_sltiu=1;}
        Reg[rt_int] = result_sltiu;
    } //sltiu
    else if (oper=="101011"){
        while(Reg[rs_int] + offset_int>=data_address+data_size*4){
            data_codes.push_back("0x0");
            data_size++;
        }
        stringstream ss; ss<< std::hex << Reg[rt_int];
        string str = ss.str();
        string str2 = "0x"; str2.append(str);
        data_codes[(Reg[rs_int]-data_address+offset_int)/4]=str2;
    } //sw
    else if (oper=="101000"){
        int rest = offset_int%4;
        while(Reg[rs_int]>data_address+data_size*4){
            data_codes.push_back("0x0");
            data_size++;
        }
        string str = data_codes[(Reg[rs_int]-data_address)/4+offset_int/4];
        while(str.length()<10){str.insert(2,"0");}
        stringstream ss; ss<< std::hex << Reg[rt_int];
        string str2 = ss.str();
        if (rest==0){str.replace(2,2,str2);}
        else if (rest ==1){str.replace(4,2,str2);}
        else if (rest ==2){str.replace(6,2,str2);}
        else if (rest ==3){str.replace(8,2,str2);}
        while(str[2] ==0){str.erase(2,1);}
        data_codes[(Reg[rs_int]-data_address)/4+offset_int/4]=str;
    } //sb
    return;
}

void j_type_oper(string str){
    string oper = detect_oper(str);
    string target = str.substr(6,26);
    int target_int = stol(target,nullptr,2);
    if (str.substr(0,6)== "000000"){
        return;
    } //r-type?
    if (oper=="000010"){
        position = ((target_int*4)-(text_address))/4;
        line_number = position;
        IF_stall=1;
    } //j
    else if (oper=="000011"){
        Reg[31] = text_address + 4*(id_inst + 1);
        position = (target_int)-(text_address/4);
        line_number = position;
        IF_stall=1;
    } //jal

    return;
}

void IF(string str){
    //IF_ID_Instr = stoi(str);
    IF_ID_NPC = position;
}

static int origine_position=0;

void ID(string str){
    string oper = detect_oper(str);
    string rs = str.substr(6,5);
    string rt = str.substr(11,5);
    string offset = str.substr(16,16);
    int offset_int = stoi(offset, nullptr, 2);
    int rs_int = stoi(rs, nullptr, 2);
    int rt_int = stoi(rt, nullptr, 2);
    ID_EX_NPC=position;
    if (detect_oper(str)=="000010" | detect_oper(str)=="000011"){

    }
    else if (detect_oper(str)=="001001"|detect_oper(str)=="001100"|detect_oper(str)=="000100"
    |detect_oper(str)=="001101"|detect_oper(str)=="001011"|detect_oper(str)=="101011"|detect_oper(str)=="101000"){
        ID_EX_rs=stoi(str.substr(6,5),nullptr, 2);
        ID_EX_rt=stoi(str.substr(11,5),nullptr, 2);
        ID_EX_Imm=stoi(str.substr(16,16),nullptr, 2);
    }
    else if (detect_oper(str)=="100011"){
        string st2 = hex_to_bi(text_codes[if_inst]);
        if(rt == st2.substr(6,5)| rt==st2.substr(11,5)){
            IF_stall=1;
            if_inst=0;
            position--;
        }
    }//lw
    else if (detect_oper(str)=="100000"){
        string st2 = hex_to_bi(text_codes[if_inst]);
        if(rt == st2.substr(6,5)| rt==st2.substr(11,5)){
            IF_stall=1;
            if_inst=0;
            position--;
        }
    }//lb
    else if (detect_oper(str)=="001000"){
        int addr = Reg[rs_int];
        position = (addr-text_address)/4;
        IF_stall=1;
        if_inst=0;
        line_number = position;
    } //jr
    if (oper=="000100"){
        if(branch_option == 1){
            origine_position = position+offset_int-1;
        }
        if(branch_option == 2){
            origine_position = position;
            position += offset_int-1;
            line_number = position;
            IF_stall=1;
        }
    } //beq
    else if (oper=="000101"){
        if(branch_option == 1){
            origine_position = position+offset_int-1;
        }
        if(branch_option ==2){
            origine_position = position;
            position += offset_int-1;
            line_number = position;
            IF_stall=1;
        }
    } //bne
    else {
        ID_EX_rs=stoi(str.substr(6,5),nullptr, 2);
        ID_EX_rt=stoi(str.substr(11,5),nullptr, 2);
        ID_EX_rd=stoi(str.substr(16,5),nullptr, 2);
    }
    j_type_oper(str);
}

void branch_stall(){
    stall=1;
    IF_stall=1;
    ID_stall=1;
    id_inst=0;
    ex_inst=0;
}

void EX(string str){
    if(str==text_codes[0]){return;}
    string oper = detect_oper(str);
    string rs = str.substr(6,5);
    string rt = str.substr(11,5);
    string offset = str.substr(16,16);
    int offset_int = stoi(offset, nullptr, 2);
    int rs_int = stoi(rs, nullptr, 2);
    int rt_int = stoi(rt, nullptr, 2);
    string st3 = hex_to_bi(text_codes[wb_inst]);
    string st2 = hex_to_bi(text_codes[mem_inst]);
    int temp_rs = Reg[rs_int];
    int temp_rt = Reg[rt_int];
    if (rs == st2.substr(16,5)|rt==st2.substr(16,5)){
            r_type_oper(st2);}
    if (rs == st3.substr(16,5)|rt==st3.substr(16,5)){
            r_type_oper(st3);}
    if (oper=="000100"){
        if(branch_option == 1){
            if(Reg[rs_int] == Reg[rt_int]){
                branch_signal=2;
            }
        }
    } //beq
    else if (oper=="000101"){
        if(branch_option ==1){
            if(Reg[rs_int] != Reg[rt_int]){
                branch_signal=2;
            }
        }
    } //bne
    else if (oper=="100011"){
        string str = data_codes[(Reg[rs_int]-data_address)/4+offset_int/4];
        string st = str.substr(2);
        int result_lw = stol(st,nullptr,16);
        EX_MEM_Alu_out = result_lw;
    }//lw
    else if (oper=="100000"){
        int rest = offset_int%4;
        int result_lb = 0;
        string str = data_codes[(Reg[rs_int]-data_address)/4+offset_int/4];
        while(str.length()<10){str.insert(2,"0");}
        if (rest==0){result_lb = stoi(str.substr(2,2),nullptr,16);}
        else if (rest ==1){result_lb = stoi(str.substr(4,2),nullptr,16);}
        else if (rest ==2){result_lb = stoi(str.substr(6,2),nullptr,16);}
        else if (rest ==3){result_lb = stoi(str.substr(8,2),nullptr,16);}
        while(str[2] ==0){str.erase(2,1);}
        EX_MEM_Alu_out = result_lb;
    }//lb
    Reg[rs_int] = temp_rs;
    Reg[rt_int] = temp_rt;
}

void MEM(string str){
    if(str==text_codes[0]){return;}
    string oper = detect_oper(str);
    string rs = str.substr(6,5);
    string rt = str.substr(11,5);
    string offset = str.substr(16,16);
    int offset_int = stoi(offset, nullptr, 2);
    int rs_int = stoi(rs, nullptr, 2);
    int rt_int = stoi(rt, nullptr, 2);
    if (oper=="000100"){
        if(branch_option ==1){
            if(Reg[rs_int] == Reg[rt_int]){
                position = origine_position;
                line_number = position;
            }
        }
        if(branch_option ==2){
            if(Reg[rs_int] == Reg[rt_int]){
            }    
            else if(Reg[rs_int] != Reg[rt_int]){
                position = origine_position-1;
                line_number = position;
                IF_stall=1;
                ID_stall=1;
                EX_stall=1;
                id_inst=0;
                ex_inst=0;
            }
        }
    } //beq
    else if (oper=="000101"){
        if(branch_option !=1){
            if(Reg[rs_int] == Reg[rt_int]){
                position = origine_position;
                line_number = position;
            }
        }
        if(branch_option ==2){
            if(Reg[rs_int] != Reg[rt_int]){
            }
            else if(Reg[rs_int] == Reg[rt_int]){
                position = origine_position-1;
                line_number = position;
                IF_stall=1;
                ID_stall=1;
                EX_stall=1;
                id_inst=0;
                ex_inst=0;
            }
        }
    } //bne
}

void WB(string str){
    r_type_oper(str);
    i_type_oper(str);
}

void Stall_manage(){
    WB_stall = MEM_stall;
    MEM_stall = EX_stall;
    EX_stall = ID_stall;
    ID_stall = IF_stall;
    IF_stall=0;
    if(stall==1){IF_stall=1;stall=0;}
}

void End_manage(){
    WB_end = MEM_end;
    MEM_end = EX_end;
    EX_end = ID_end;
    ID_end = IF_end;
}

void Inst_manage(){
    wb_inst = mem_inst;
    mem_inst = ex_inst;
    ex_inst = id_inst;
    id_inst = if_inst;
}




int main(int argc, char* argv[]){

    std::ifstream file_1;
    if (argc == 1){return 0;}
    string input_f = argv[argc - 1];
    if (input_f.substr(input_f.length()-2,2)!=".o"){cout<<"Error : You have to command with input file!"<<endl;return 0;}
	file_1.open(argv[argc - 1]); 

    string memory_start;
    string memory_end;
    int counts_limit = 0;
    int instruction_counts = 0;

    //ifstream file_1;
    //file_1.open("sample.o");EMSTALL in cycle1

    for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-m")) {
			input_option_m = 1;
            string range(argv[i+1]);
            memory_start = range.substr(0,range.find(":"));
            memory_end = range.substr(range.find(":")+1);
		}
		if (!strcmp(argv[i], "-d")) {
            input_option_d = 1;
		}
		if (!strcmp(argv[i], "-n")) {
			input_option_n = 1;
            string counts(argv[i+1]);
            counts_limit = stoi(counts);
		}
        if (!strcmp(argv[i], "-antp")) {
			branch_option = 1;
            string counts(argv[i+1]);
		}
        if (!strcmp(argv[i], "-atp")) {
			branch_option = 2;
            string counts(argv[i+1]);
		}
        if (!strcmp(argv[i], "-p")) {
			input_option_p = 1;
            string counts(argv[i+1]);
		}
	}

    if (branch_option ==0){cout<<"Error : You have to command with atp or antp!"<<endl; file_1.close(); return 0;}

    for(int i=0; i<32; i++){ Reg.push_back(0);};// make register $0 ~ $31
    
    if (file_1.is_open()){
        while (!file_1.eof()){
            string str;
            getline(file_1, str);
            if (position ==0 & str.empty()){text_size=0; data_size=0;break;}
            if (position == 0){text_size = stoi(str.substr(2,str.length()),nullptr,16)/4; } //text size 
            else if (position == 1){data_size = stoi(str.substr(2,str.length()),nullptr,16)/4;} //data size
            else if (position >1 && position < text_size+2 ){text_codes.push_back(str);}// text section --> check operator from codes
            else if (position > text_size+1){data_codes.push_back(str);}// data section
            position++;
            if (position == text_size+data_size+2){break;} //line check done
        }
    }
    position = 0;

    while (position < text_size+5){
        if (input_option_n==1 & instruction_counts>=counts_limit){cycle++;break;}
        if (position>text_size) {IF_end =1;}
        if (position >= text_size){stall=1;}
        if (position>=0){
            Stall_manage();
            cycle++;
            if (IF_end==1){
                if_inst=0;
            }
            else if(IF_stall!=0){
                if_inst = 0;
            }
            else{
                IF(text_codes[position]);
                if_inst = position;
                position++;
                line_number = position;
            }
        }
        if (line_number-1>0){
            if(ID_end==1){
            }
            else if(ID_stall!=0){
                id_inst=0;
            }
            else{
                ID(hex_to_bi(text_codes[id_inst]));
            }
        }
        if (line_number-2>0){
            if(EX_end==1){
            }
            else if(EX_stall != 0){
            }
            else{
                EX(hex_to_bi(text_codes[ex_inst]));
            }
        }
        if (line_number-3>0){
            if(MEM_end==1){
            }
            else if(MEM_stall != 0){
            }
            else{
                MEM(hex_to_bi(text_codes[mem_inst]));
            }
        }
        if (line_number-4>0){
            if(WB_end==1){
                break;
            }
            else if(WB_stall != 0){
            }
            else{
                WB(hex_to_bi(text_codes[wb_inst]));
                instruction_counts++;
            }
            
        }

        if (cycle!=1 & if_inst==0 & id_inst==0 & ex_inst==0 & mem_inst==0 & wb_inst==0){break;}
        ///pipeline print part///
        if (input_option_d == 1){
            cout <<endl<< "===== Cycle " << cycle << " =====" <<endl;
            if (input_option_p ==1){
            cout<<"Current pipeline PC state:"<<endl;
            cout<<"{";
            int arr[5] = {if_inst,id_inst,ex_inst,mem_inst,wb_inst};
            for(int a=0; a<5; a++){
                if(a==4){
                    if(arr[a]!=0){
                    stringstream as; as<< std::hex << PC+(arr[a]*4);
                    cout<<"0x"<<as.str();
                    }
                }
                else if(arr[a]==0 & a==cycle-1){
                    stringstream as; as<< std::hex << PC+(arr[a]*4);
                    cout<<"0x"<<as.str()<<"|";
                }
                else if(arr[a]==0){
                    cout<<"|";
                }
                else {
                    stringstream as; as<< std::hex << PC+(arr[a]*4);
                    cout<<"0x"<<as.str()<<"|";
                }
            }cout<<"}"<<endl;}
            ///register print part///
            cout<<endl<< "Current register values:" << endl << "----------------------------------" <<endl;
            stringstream sss; sss<< std::hex << PC+(line_number*4); 
            cout << "PC: " << sss.str() <<endl;
            cout<<"Registers:"<<endl;
            for(int i=0; i<32; i++){ 
                cout<<"R" << i <<": 0x";
                stringstream ss; ss<< std::hex << Reg[i]; cout<<ss.str()<<endl; 
                }; cout<<endl;
            if(input_option_m == 1){
                int mem_start = stoi(memory_start,nullptr,16);
                int mem_end = stoi(memory_end,nullptr,16);
                stringstream ss_1; ss_1<< std::hex << mem_start;
                stringstream ss_2; ss_2<< std::hex << mem_end;
                cout << "Memory content [0x" << ss_1.str() << "..0x"<< ss_2.str() <<"]:"<< endl << "----------------------------------" <<endl;
                for(int i=mem_start; i<=mem_end; i+=4){ 
                    cout<<"0x";
                    stringstream ss; ss<< std::hex << i; cout<<ss.str();
                    if (i>=text_address & i <= text_address+4*32 ){
                        stringstream ss; ss<< std::hex << text_codes[(i-text_address)/4]; string ss_s = ss.str();
                        while(ss_s[2]=='0'){ss_s.erase(2,1);}
                        cout<<": "<<ss_s<<endl;
                    }
                    else if (i>=data_address & i < data_address+4*data_size){
                        stringstream ss; ss<< std::hex << data_codes[(i-data_address)/4]; string ss_s = ss.str();
                        while(ss_s[2]=='0'){ss_s.erase(2,1);}
                        cout<<": "<<ss_s<<endl;
                    }
                    else{
                    cout<<": 0x0"<<endl;
                    }
                };
            }
        }
        if(branch_signal>1){branch_signal--;}
        else if(branch_signal==1){branch_stall();}
        Inst_manage();
        End_manage();
    }// data section --> check operator from codes


//print part
    if (input_option_d==0 |input_option_d==1| counts_limit==0 | (text_size==0 & data_size==0)){
        cout <<endl<< "===== Completion Cycle: " << cycle-1 << " =====" <<endl;
            cout<<"Current pipeline PC state:"<<endl;
            cout<<"{";
            int arr[5] = {if_inst,id_inst,ex_inst,mem_inst,wb_inst};
            for(int a=0; a<5; a++){
                if(a==4){
                    if(arr[a]!=0){
                    stringstream as; as<< std::hex << PC+(arr[a]*4);
                    cout<<"0x"<<as.str();
                    }
                }
                else if(arr[a]==0 & a==cycle-1){
                    stringstream as; as<< std::hex << PC+(arr[a]*4);
                    cout<<"0x"<<as.str()<<"|";
                }
                else if(arr[a]==0){
                    cout<<"|";
                }
                else {
                    stringstream as; as<< std::hex << PC+(arr[a]*4);
                    cout<<"0x"<<as.str()<<"|";
                }
            }cout<<"}"<<endl;
        cout <<endl<< "Current register values:" << endl << "----------------------------------" <<endl;
        stringstream sss; sss<< std::hex << PC+(position*4); 
        cout << "PC: " << sss.str() <<endl;
        cout<<"Registers:"<<endl;
        for(int i=0; i<32; i++){ 
            cout<<"R" << i <<": 0x";
            stringstream ss; ss<< std::hex << Reg[i]; cout<<ss.str()<<endl; 
            }; cout<<endl;

        if(input_option_m == 1){
            int mem_start = stoi(memory_start,nullptr,16);
            int mem_end = stoi(memory_end,nullptr,16);
            stringstream ss_1; ss_1<< std::hex << mem_start;
            stringstream ss_2; ss_2<< std::hex << mem_end;
            cout << "Memory content [0x" << ss_1.str() << "..0x"<< ss_2.str() <<"]:"<< endl << "----------------------------------" <<endl;
            for(int i=mem_start; i<=mem_end; i+=4){ 
                cout<<"0x";
                stringstream ss; ss<< std::hex << i; cout<<ss.str();
                if (i>=text_address & i <= text_address+4*32 ){
                    stringstream ss; ss<< std::hex << text_codes[(i-text_address)/4]; string ss_s = ss.str();
                        while(ss_s[2]=='0'){ss_s.erase(2,1);}
                        cout<<": "<<ss_s<<endl;
                }
                else if (i>=data_address & i < data_address+4*data_size){
                    stringstream ss; ss<< std::hex << data_codes[(i-data_address)/4]; string ss_s = ss.str();
                        while(ss_s[2]=='0'){ss_s.erase(2,1);}
                        cout<<": "<<ss_s<<endl;
                }
                else{
                cout<<": 0x0"<<endl;
                }
            };
    }}
    

    file_1.close();
    return 0;

}