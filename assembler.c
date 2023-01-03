#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define DATA_SEC_ADDR 0x10000000
#define TEXT_SEC_ADDR 0x00400000
#define DATA_SEC_SIZE 0x00001000//0x00002000
#define TEXT_SEC_SIZE 0x00001000//0x03f00000

int Words[DATA_SEC_SIZE]={0};
int Word_Addrs[DATA_SEC_SIZE];
char Word_Labels[DATA_SEC_SIZE+1][16];

int Texts[TEXT_SEC_SIZE] = {0};
int Text_Addrs[TEXT_SEC_SIZE];
char Text_Labels[TEXT_SEC_SIZE+1][16];

int str2int (char *str) {	
	int radix = 10;
	char *hex = "xX";
    if (sizeof(str) > sizeof(char) && strchr(hex, str[1])!=NULL)
        radix = 16;
	return (int) strtol(str, NULL, radix);
}

int lower16bit (int a) {
	return a & (int)pow(2, 16) - 1;
}

int tlabel2addr (char *label) {
	for (int i = 0; i < TEXT_SEC_SIZE; i++) {
		if( strcmp(Text_Labels[i], label) == 0 )
			return Text_Addrs[i];
	}
	return 0;
}

int wlabel2addr (char *label) {
	for (int i = 0; i < DATA_SEC_SIZE; i++){
		if( strcmp(Word_Labels[i], label) == 0 )
			return Word_Addrs[i];
	}
	return 0;
}

int main(int argc, char *argv[]) {
	//File load
	FILE *fp = fopen(argv[1], "r");
	int size;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	
	char buffer[size+1];
	memset(buffer, 0, size+1);
	
	fseek(fp, 0, SEEK_SET);
	fread(buffer, size, 1, fp);
	fclose(fp);

	//Addrs[] setup
	for(int i = 0; i < DATA_SEC_SIZE; i++)
		Word_Addrs[i] = DATA_SEC_ADDR + i*4;
	
	for(int i = 0; i < TEXT_SEC_SIZE; i++)
		Text_Addrs[i] = TEXT_SEC_ADDR + i*4;



	//Make R, I, J format
	char *R_format[] = {"addu", "and", "jr", "nor", "or", "sltu", "sll", "srl", "subu"};
	char *R_funct_str[] = {"0x21", "0x24", "0x8", "0x27", "0x25", "0x2b", "0x0", "0x2", "0x23"};
	int R_size = sizeof(R_format)/sizeof(R_format[0]);
	int R_funct[9];
	for (int i = 0; i < 9; i++)
		R_funct[i] = (int) strtol(R_funct_str[i], NULL, 16);
	
	char *I_format[] = {"addiu", "andi", "ori", "sltiu",
						"sw", "sb", "lb", "lw",
						"beq", "bne",
						"lui"};
	char *I_op_str[] = {"0x9", "0xc", "0xd", "0xb",
						"0x2b", "0x28", "0x20", "0x23",
						"0x4", "0x5",
						"0xf"};
	int I_size = sizeof(I_format)/sizeof(I_format[0]);
	int I_op[11];
	for (int i = 0; i < 11; i++)
		I_op[i] = (int) strtol(I_op_str[i], NULL, 16);
	
	char *J_format[] = {"j", "jal"};
	int J_op[] = {2, 3};
	int J_size = sizeof(J_format)/sizeof(J_format[0]);
	
	
	//Save words in Words[], word labels in Word_Labels[]
	char *ptext = strstr(buffer, ".text");
	char *deli = " ,$()\n";	//delimites for strtok()
	char *tok = strtok(buffer, deli);	//tok = ".data" or ".text"
	int windex = 0; //Words index
	//if there are no words, tok == ptext, then skip while loop.
	while ( tok < ptext) {
		tok = strtok(NULL, deli);
		if ( strcmp(tok, ".word") == 0 )
			Words[windex++] = str2int(strtok(NULL, deli));
		else {
			strcpy(Word_Labels[windex], tok);
			Word_Labels[windex][strlen(tok) - 1] = '\0'; //remove ':'
		}
	}
	//now, tok = ".text"
	
	//Text label addresses are not saved yet, so they will be processed later.
	//Save where to jump
	int Jump_Indexs[TEXT_SEC_SIZE] = {0}; //index of instructions that need to jump
	char Jump_Labels[TEXT_SEC_SIZE][16]; //Label to jump
	int jindex = 0;	//number of jump
    
	//Save intsructions in Texts[], labels in Text_Labels[]
	tok = strtok(NULL, deli); //tok = "main:" or command
	int tindex = 0; //Texts index
	while (tok){
		//Save a text label
		if (strchr(tok, ':') != NULL){
			strcpy(Text_Labels[tindex], tok);
			Text_Labels[tindex][strlen(tok) - 1] = '\0'; //remove ':'
			tok = strtok(NULL, deli);
			continue;
		}
		//R format
		for (int i = 0; i < R_size; i++){
			if (strcmp(R_format[i], tok) == 0){
				//Texts[tindex][0] = 0;	//op
				Texts[tindex] += R_funct[i];
				if ( !strcmp(tok, "sll") || !strcmp(tok, "srl")){
					//Texts[tindex][1] = 0;	//rs
					Texts[tindex] += str2int(strtok(NULL, deli)) << 11;//rd
					Texts[tindex] += str2int(strtok(NULL, deli)) << 16;//rt
					Texts[tindex] += str2int(strtok(NULL, deli))<<6;//shamt
				}
				else if ( !strcmp(tok, "jr") ){
					Texts[tindex] += str2int(strtok(NULL, deli))<<21;//rs
				}
				else {
					Texts[tindex] += str2int(strtok(NULL, deli))<<11;//rd
					Texts[tindex] += str2int(strtok(NULL, deli))<<21;//rs
					Texts[tindex] += str2int(strtok(NULL, deli))<<16;//rt
				}
			}
		}
		//I format
		for (int i = 0; i < I_size; i++){
			if(strcmp(I_format[i], tok) == 0){	
				Texts[tindex] += I_op[i]<<26;	//op
				//addiu, andi, ori, sltiu
				if ( i < 4 ){
					Texts[tindex] += str2int(strtok(NULL, deli))<<16;//rt
					Texts[tindex] += str2int(strtok(NULL, deli))<<21;//rs
					Texts[tindex] += lower16bit(str2int(strtok(NULL, deli)));	//imm
				}
				//lb, lw, sw, sb
				else if ( i >= 4 && i < 8 ){
					Texts[tindex] += str2int(strtok(NULL, deli))<<16;//rt
					Texts[tindex] += lower16bit(str2int(strtok(NULL, deli)));//offset
					Texts[tindex] += str2int(strtok(NULL, deli))<<21;//rs
				}
				//beq, bne
				else if ( i >= 8 && i < 10 ){
					Texts[tindex] += str2int(strtok(NULL, deli))<<21;//rs
					Texts[tindex] += str2int(strtok(NULL, deli))<<16;//rt
					strcpy(Jump_Labels[jindex], strtok(NULL, deli));
					Jump_Indexs[jindex++] = tindex;
					//offset
				}
				//lui
				else {
					//Texts[tindex] += 0<<21;	//rs
					Texts[tindex] += str2int(strtok(NULL, deli))<<16;//rt
					Texts[tindex] += lower16bit(str2int(strtok(NULL, deli))); //imm

				}
			}
		}
		//J format
		for (int i = 0; i < J_size; i++){
			if ( strcmp(J_format[i], tok) == 0){
				Texts[tindex] += J_op[i]<<26;	//op
				strcpy(Jump_Labels[jindex], strtok(NULL, deli));
				Jump_Indexs[jindex++] = tindex;
				//target
			}
		}

		//La
		if ( strcmp("la", tok) == 0 ){
			int rt = str2int(strtok(NULL, deli));	//register#
			int imm = wlabel2addr(strtok(NULL, deli));	//address
			//lui
			Texts[tindex] += 0xf<<26;	//op
			Texts[tindex] += rt<<16;
			Texts[tindex] += imm>>16;//upper 16bit of address
			//ori
			int lowerimm = lower16bit(imm);
			if ( lowerimm ){
				tindex++;
				Texts[tindex] += 0xd<<26;	//op
				Texts[tindex] += rt<<21;	//rs == rt
				Texts[tindex] += rt<<16;	//rt
				Texts[tindex] += lowerimm;
				//imm & 1111 1111 1111 1111
			}
		}
		
		tok = strtok(NULL, deli);
		tindex++;
	}
	
	//Handling missing jump labels
	for (int i = 0; i < jindex; i++) {
		int addr = tlabel2addr(Jump_Labels[i]);
		int ji = Jump_Indexs[i];
		//j(2), jar(3) : target = addr / 4
		if ((Texts[ji] >> 26) < 4)
			Texts[ji] += addr / 4;
		//beq(4), bne(5) : offset = (addr - PC)/4 - 1
		else{
			int offset = (addr - Text_Addrs[ji])/4 - 1;
			//convert offset to 16bit
			Texts[ji] += lower16bit(offset);
		}
	}

	char *fname = argv[1];
	fname[strlen(fname) - 1] = 'o';
	FILE *objfp = fopen(fname, "w");
	fprintf(objfp, "0x%x\n", tindex*4);
    fprintf(objfp, "0x%x\n", windex*4);
    for(int i = 0; i < tindex; i++)
		fprintf(objfp, "0x%x\n", Texts[i]);
    for(int i = 0; i < windex; i++)
		fprintf(objfp, "0x%x\n", Words[i]);
	fclose(objfp);

	return 0;
}
