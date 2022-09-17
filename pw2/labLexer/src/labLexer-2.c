// relop lexer
#include <stdlib.h>
#include <string.h>
#include "pl0.h"

void Print0(int num)
{
	printf("(other,%d)", num);
}

void Print(int id)
{
	switch (id) {
	case 2: printf("(relop,<=)"); break;
	case 3: printf("(relop,<>)"); break;
	case 1: printf("(relop,<)"); break;
	case 5: printf("(relop,=)"); break;
	case 7: printf("(relop,>=)"); break;
	case 6: printf("(relop,>)"); break;
	}
}

#ifndef LEXERGEN
char lines[1024]={0};
int index_line=0; // record the index of line of my code
int state=0;

void getsym(){
    while(1){
    	if(lines[index_line]=='\n' || lines[index_line]=='\r'){
    		if(state==0 && strlen(id)) sym = 0;
			else if(state){
				sym = state; state = 0; 
			}
			else sym = 10; 
			return;
		}
		int i = index_line;
		switch (state) {
		case 0:
			if (lines[i] == '<' || lines[i] == '=' || lines[i] == '>') {
				if (strlen(id)) {
					sym=0; return;
				}
				if (lines[i] == '<') state = 1;
				else if (lines[i] == '=') state = 5;
				else if (lines[i] == '>') state = 6;
			}
			else {
				id[strlen(id)+1]='\0';
				id[strlen(id)]=lines[i];
			}

			break;

		case 1:
			if (lines[i] == '=') state = 2;
			else if (lines[i] == '>') state = 3;
			else {
				sym=state; state=0; return;
			}
			break;

		case 2:
			sym=state; state=0; return;
			break;

		case 3:
			sym=state; state=0; return;
			break;

		case 5:
			sym=state; state=0; return;
			break;

		case 6:
			if (lines[i] == '=') state = 7;
			else {
				sym=state; state=0; return;
			}
			break;

		case 7:
			sym=state; state=0; return;
			break;
		}
		index_line++;
	}
}
#endif

int main(int argc, char *argv[])
{
#ifdef LEXERGEN
	extern FILE * yyin;
	yyin=stdin;
#else
	fgets(lines, 1024, stdin);
#endif
	getsym();	
	while (1) {
		if(sym==10) break;
		else if(sym==0){
			Print0(strlen(id));
			strcpy(id,"");
		}
		else{
			int tmp=sym;
			Print(tmp);
		}
		
		getsym();
	}
	return 0;
}
