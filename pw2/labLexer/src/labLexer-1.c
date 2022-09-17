#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

int main()
{
	char line[1024]={0};
	fgets(line, 1024, stdin);
	int state = 0;
	int num_nop = 0;
	int length = strlen(line);

	for (int i = 0; i < length; i++) {
		if(line[i]=='\n' || line[i]=='\r'){
			if(state==0 && num_nop)  Print0(num_nop);
			if(state) Print(state);
			break;
		}
		switch (state) {

		case 0:
			if (line[i] == '<' || line[i] == '=' || line[i] == '>') {
				if (num_nop) {
					Print0(num_nop);
					num_nop = 0;
				}
				if (line[i] == '<') state = 1;
				else if (line[i] == '=') state = 5;
				else if (line[i] == '>') state = 6;
			}
			else num_nop++;
			break;

		case 1:
			if (line[i] == '=') state = 2;
			else if (line[i] == '>') state = 3;
			else {
				Print(state); state = 0; i--;
			}
			break;

		case 2:
			Print(state); state = 0; i--;
			break;

		case 3:
			Print(state); state = 0; i--;
			break;

		case 5:
			Print(state); state = 0; i--;
			break;

		case 6:
			if (line[i] == '=') state = 7;
			else {
				Print(state); state = 0; i--;
			}
			break;

		case 7:
			Print(state); state = 0; i--;
			break;
		}
	}
	return 0;
}