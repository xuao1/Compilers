#include <iostream>
#include <cstring>
#include <cstdlib>

#include "antlr4-runtime.h"
#include "relop.h"

using namespace antlr4;

int main(int argc, const char* argv[]) {
    //ANTLRInputStream input("if (a>=b) a=b\n");
    char lines[1024];
    fgets(lines, 1024, stdin);
    ANTLRInputStream input(lines);
    relop lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
        if (token->getType() == -1) {
            break;
        }
        else if (token->getType() == 31) {
            std::cout << "(other," << token->getText().length() << ")";
        }
        else {
            std::cout << "(relop," << token->getText() << ")";
        }
    }

    return 0;
}