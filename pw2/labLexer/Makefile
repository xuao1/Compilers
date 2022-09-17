CC       = gcc -g
LEX      = flex -i -I 
INC      = -I../pl0/include 

all: reloplex relop

reloplex:   grammar/relop.lex
	cd grammar; $(LEX)  relop.lex
	$(CC)  $(INC) -DLEXERGEN grammar/lex.yy.c src/labLexer-2.c -o labLexer-2 -ll

relop:   src/labLexer-2.c
	$(CC)  $(INC) src/labLexer-2.c -o labLexer-2m -ll

clean:
	rm -f *.BAK *.o core *~* *.a 
	rm -f *.tab.h *.tab.c
	rm -f relop *.out