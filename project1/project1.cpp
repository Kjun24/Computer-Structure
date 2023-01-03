#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <stdlib.h>

using namespace std;
class R_type;
class I_type;
class J_type;

vector<string> encoded_binary;
static int data_section_size = 0;
static int text_section_size = 0;

class R_type {
    public:
    int op=0,rs,rt,rd,shamt,funct=0;
    string oper,  binarycode;
    int get_oper(string input);
    R_type(string r_op,int var1, int var2, int var3);
    string get_binarycode();
};

int R_type :: get_oper(string input){
    if (input=="addu"){return 33;}
    else if (input == "and"){return 36;}
    else if (input == "jr"){return 8;}
    else if (input == "nor"){return 39;}
    else if (input == "or"){return 37;}
    else if (input == "sltu"){return 43;}
    else if (input == "sll"){return 0;}
    else if (input == "srl"){return 2;}
    else if (input == "subu"){return 35;}
    else {return 0;}
}

R_type :: R_type(string r_op,int var1, int var2, int var3){
    if (r_op == "jr"){
    
        oper = r_op;
        rd = 0;
        rs = var1;
        rt = 0;
        shamt = 0;
    }
    else if(r_op == "sll"){
        oper = r_op;
        rd = var1;
        rs = 0;
        rt = var2;
        shamt = var3;
    }
    else if(r_op == "srl"){
        oper = r_op;
        rd = var1;
        rs = 0;
        rt = var2;
        shamt = var3;
    }
    else {
        oper = r_op;
        rd = var1;
        rs = var2;
        rt = var3;
        shamt = 0;
    }
    funct = get_oper(r_op);
    string a = bitset<6>(op).to_string();
    string b = bitset<5>(rs).to_string();
    string c = bitset<5>(rt).to_string();
    string d = bitset<5>(rd).to_string();
    string e = bitset<5>(shamt).to_string();
    string f = bitset<6>(funct).to_string();
    binarycode = a+b+c+d+e+f;
    cout << "binary : " << binarycode << endl;
	bitset<32> he(binarycode);	
    bitset<32> abc = he;
	cout <<"0x"<< hex << he.to_ulong() << endl;
    encoded_binary.push_back(binarycode);
}
string R_type::get_binarycode(){
    return binarycode;
}

class I_type {
    public:
    int op,rs,rt,imme;
    string oper, binarycode;
    I_type(string r_op,int var1, int var2, int var3);
    int get_oper(string input);
    string get_binarycode();
};

int I_type :: get_oper(string input){
    if (input=="addiu"){return 9;}
    else if (input == "andi"){return 12;}
    else if (input == "beq"){return 4;}
    else if (input == "bne"){return 5;}
    else if (input == "lui"){return 15;}
    else if (input == "lw"){return 35;}
    else if (input == "lb"){return 32;}
    else if (input == "ori"){return 13;}
    else if (input == "sltiu"){return 11;}
    else if (input == "sw"){return 43;}
    else if (input == "sb"){return 40;}
    else {return 0;}
}

I_type :: I_type(string r_op,int var1, int var2, int var3){
    oper = r_op;
    op = get_oper(r_op);
    rs = var2;
    rt = var1;
    imme = var3;
    string a = bitset<6>(op).to_string();
    string b = bitset<5>(rs).to_string();
    string c = bitset<5>(rt).to_string();
    string d = bitset<16>(imme).to_string();
    binarycode = a+b+c+d;
    cout << "binary : " << binarycode << endl;
	bitset<32> he(binarycode);	
	cout <<"0x"<< hex << he.to_ulong() << endl;
    encoded_binary.push_back(binarycode);
}

string I_type::get_binarycode(){
    return binarycode;
}

class J_type {
    public:
    int op,target_address;
    string oper, binarycode;
    int target;
    J_type(string r_op, int var1);
    int get_oper(string input);
    string get_binarycode();
};

int J_type :: get_oper(string input){
    if (input=="j"){return 2;}
    else if (input == "jal"){return 3;}
    else {return 0;}
}

string J_type::get_binarycode(){
    return binarycode;
}

J_type :: J_type(string r_op, int var1){
    oper = r_op;
    op = get_oper(r_op);
    target = var1;
    string a = bitset<6>(op).to_string();
    string b = bitset<26>(target).to_string();
    binarycode = a+b;
    cout << "binary : " << binarycode << endl;
	bitset<32> he(binarycode);	
	cout <<"0x"<< hex << he.to_ulong() << endl;
    encoded_binary.push_back(binarycode);
}

vector<string> text_reading(string input_str){
    vector<string> v;
    char blank = ' ';
    string hexa = "0x";
    int num;
    input_str.erase(0,4);
    if (input_str.front() == blank){input_str.erase(0,4);}
    if (input_str[input_str.find(' ')+1]==blank){num = 8;}
    else { num = input_str.find(' ')+1;}
    string oper = input_str.substr(0,num);
    oper.erase(std::remove(oper.begin(), oper.end(), ' '), oper.end());
    v.push_back(oper); input_str.erase(0,num);
    int len=input_str.length();
    while (input_str.length()>0){
        if(input_str.find(',')<input_str.length()){
            int k = input_str.find(",");
            if(input_str.find('$')<input_str.length()){v.push_back(input_str.substr(1, k-1));}
            else {v.push_back(input_str.substr(0, k-1));}
            input_str.erase(0, k+2);}
        else {
            input_str.erase(std::remove(input_str.begin(), input_str.end(), ' '), input_str.end());
            if(input_str.find('$')<input_str.length()&&input_str.find('(')>input_str.length()){input_str.erase(0, 1); v.push_back(input_str);}
            else {if(input_str.find(hexa)<input_str.length()){
                input_str.erase(0,2);
                int x;   
                std::stringstream ss;
                ss << std::hex << input_str;
                ss >> x; 
                v.push_back(to_string(x));}
                else if(input_str.find('(')<input_str.length()){
                    string num = input_str.substr(0,input_str.find('('));
                    input_str.erase(0,input_str.find('(')+2); input_str.erase(input_str.size()-1,1);
                    v.push_back(input_str);
                    v.push_back(num);
                }
                else{v.push_back(input_str);}}
            input_str.erase();
        }
    }
    return v;
}

void la_define(string op , int var1, int var2) {
    int up = var2 & 0xffff0000;
    int down = var2 & 0x0000ffff;
    bitset<16>bit1(var2); bitset<16>bit2(0); bitset<16>bit = bit1 | bit2;
    if (bit == 0) {
	    bitset<32>bit1(up>>16);    	
        I_type isa("lui",var1, 0, bit1.to_ulong());
    }
    else {
        bitset<32>bit1(up>>16);
        bitset<32>bit2(down);	
        I_type isa("lui", var1, 0, bit1.to_ulong());
        I_type isa2("ori", var1, var1, bit2.to_ulong());
        text_section_size++;
    }
    return;
}

int main(int argc, char**argv){
    int data_start_address = 0x10000000;
    int text_start_address = 0x400000;

    bool data_section = false;
    bool text_section = false;

    vector<string> data_vec;
    vector<string> data_vari_vec;
    int data_vari_num=0;

    vector<vector<string>> text_vector;
    vector<string> text_labels;
    int text_labels_num;
    
    vector<string> Itype_list = {"addiu","andi","beq","bne","lui","lw","ori","sltiu","sw","lb","sb","la"};
    vector<string> Rtype_list = {"addu","and","jr","nor","or","sltu","sll","srl","subu"};
    vector<string> Jtype_list = {"j","jal"};

    std::ifstream file_1;
    file_1.open(argv[1]);

    //ifstream file_1;
    //file_1.open("sample.s");
    if (file_1.is_open()){
        while (!file_1.eof()){
            string str;
            getline(file_1, str);
            if(str.find(".data")<str.length()){data_section = true; text_section = false;} //detecing data section
            if(str.find(".text")<str.length()){data_section = false; text_section = true; str.erase(std::remove(str.begin(), str.end(), ' '), str.end());} //detecting text section
            if(data_section && str.find(".word")<str.length()){
                if (str.find(":")<str.length()){ 
                    data_vari_num++;
                    data_vari_vec.push_back(str.substr(0,str.find(":")));
                    data_vari_vec.push_back(to_string(data_section_size));
                    data_vec.push_back(str.substr(str.find(".word")+8));}
                else {data_vec.push_back(str.substr(str.find(".word")+8));}
                data_section_size++;
            } //data section analyzing
            if(text_section &&  str != ".text" && str!=""){
                if (str.find(":")==str.length()-1){ 
                    text_labels.push_back(str.substr(0,str.find(":")));
                    text_labels.push_back(to_string(text_section_size));
                    str.erase(0,str.find(":")+1);
                    str.insert(0, "   ");}
                else if (str.find(":")<str.length()-1){
                    text_labels.push_back(str.substr(0,str.find(":")));
                    text_labels.push_back(to_string(text_section_size));
                    str.erase(0,str.find(":")+1);
                    str.insert(0, "   ");
                    vector<string> v2 = text_reading(str);
                    text_vector.push_back(v2);
                    text_section_size++;}
                else {vector<string> v2 = text_reading(str);
                    text_vector.push_back(v2);
                    text_section_size++;}
            } //text section analyzing
        };
        int text_section_length = text_section_size;

        for(int i=0; i < text_section_length; i++){
            vector<string> decode_vec = text_vector[i];
            auto it_I = find(Itype_list.begin(),Itype_list.end(),decode_vec[0]);
            if (it_I!=Itype_list.end()){
                string oper = decode_vec[0];
                if (oper=="lui"){
                    int var1 = stoi(decode_vec[1]);
                    int var2 = 0;
                    int var3 = stoi(decode_vec[2]);
                    I_type isa(oper, var1, var2, var3);}
                else if (oper == "la"){
                    int var1 = stoi(decode_vec[1]);
                    int var2 = 0;
                    auto it_data = find(data_vari_vec.begin(),data_vari_vec.end(),decode_vec[2]);
                    string variable_index;
                    for (int a =0; a<data_vari_vec.size(); a++){
                        if (data_vari_vec[a] == decode_vec[2]) {variable_index = data_vari_vec[a+1]; break;}
                    }
                    int variable_address = data_start_address + 4*(stoi(variable_index));
                    la_define(oper , var1 , variable_address);
                }
                else if (oper == "bne"){
                    int var1 = stoi(decode_vec[2]);
                    int var2 = stoi(decode_vec[1]);
                    string text_label;
                    for (int a =0; a<text_labels.size(); a++){
                        if (text_labels[a] == decode_vec[3]) {text_label = text_labels[a+1]; break;}
                    }
                    int label_address = text_start_address + 4*(stoi(text_label));
                    int bne_address = text_start_address + 4*i;
                    int offset = label_address - 4 - bne_address;
                    I_type isa(oper, var1, var2, offset/4);
                }
                else if (oper == "beq"){
                    int var1 = stoi(decode_vec[2]);
                    int var2 = stoi(decode_vec[1]);
                    string text_label;
                    for (int a =0; a<text_labels.size(); a++){
                        if (text_labels[a] == decode_vec[3]) {text_label = text_labels[a+1]; break;}
                    }
                    int label_address = text_start_address + 4*(stoi(text_label));
                    int beq_address = text_start_address + 4*i;
                    int offset = label_address - 4 - beq_address;
                    I_type isa(oper, var1, var2, offset/4);
                }
                else {int var1 = stoi(decode_vec[1]);
                    int var2 = stoi(decode_vec[2]);
                    int var3 = stoi(decode_vec[3]);
                    I_type isa(oper, var1, var2, var3);}
            }
            auto it_R = find(Rtype_list.begin(),Rtype_list.end(),decode_vec[0]);
            if (it_R!=Rtype_list.end()){
                string oper = decode_vec[0];
                if(oper =="jr"){
                int var1 = stoi(decode_vec[1]);
                int var2 = 0;
                int var3 = 0;
                R_type isa(oper, var1, var2, var3);}
                else {
                int var1 = stoi(decode_vec[1]);
                int var2 = stoi(decode_vec[2]);
                int var3 = stoi(decode_vec[3]);
                R_type isa(oper, var1, var2, var3);}
            }
            auto it_J = find(Jtype_list.begin(),Jtype_list.end(),decode_vec[0]);
            if (it_J!=Jtype_list.end()){
                string oper = decode_vec[0];
                string text_label;
                for (int i =0; i<text_labels.size(); i++){
                    if (text_labels[i] == decode_vec[1]) {text_label = text_labels[i+1]; break;}
                }
                int text_address = text_start_address + 4*(stoi(text_label));
                J_type isa(oper, text_address/4);}
        }
    }

    if (argc>1){
    string input_name(argv[1]);
    string output_name;
    if(input_name.find(".s")){
        input_name.replace(input_name.find(".s"), 2, ".o");
        output_name = input_name;
    }

    std::ofstream fout(output_name, std::ios::out|std::ios::binary);
    
    std::stringstream de;
    de<< std::hex << text_section_size*4;
    std::string out1 ( de.str() );
    fout<<"0x"<<out1<<endl; // text section size fout
    std::stringstream dec;
    dec<< std::hex << data_section_size*4;
    std::string out2 ( dec.str() );
    fout<<"0x"<<out2<<endl; // data section size fout
    
    for(int i=0; i<encoded_binary.size(); i++){
        string bcode = encoded_binary[i];
	    bitset<32> he(bcode);	
	    fout <<"0x"<< hex << he.to_ulong() << endl;
    }
    for(int i=0; i<data_vec.size(); i++){
        if(data_vec[i][1] == 'x'){fout<<data_vec[i]<<endl;}
        else{
        string bcode = bitset<32>(stoi(data_vec[i])).to_string();
	    bitset<32> he(bcode);	
	    fout <<"0x"<< hex << he.to_ulong() << endl;}
    }
    fout.close();
    }
    file_1.close();
    cout<<"Decoding is done"<<endl;
    
    return 0;
}
