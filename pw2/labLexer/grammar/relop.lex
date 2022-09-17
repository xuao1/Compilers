/*
 * relop.lex : Scanner for a simple
 *            expression parser.
 */

%{
#include "pl0.h"
#include <stdio.h>
#include <string.h>
%}

%%

">"         {return(6);}
">="        {return(7);}
"="         {return(5);}
"<"         {return(1);}
"<>"        {return(3);}
"<="        {return(2);}
[\n\r]      {return(10);}
[^<=>\n\r]+  {
    strcpy(id,yytext);
    return(0);
}


%%
void getsym()
{
	sym = yylex();
}