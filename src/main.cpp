#include <cassert>
#include <cstdio>
#include <iostream>
#include "ast.h"

using namespace std;

/* lexer input */
extern FILE *yyin;

/* bison parser */
extern int yyparse(BaseAST*& ast);

int main(int argc, const char *argv[]) {
    assert(argc == 5);

    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    yyin = fopen(input, "r");
    assert(yyin);

    BaseAST* ast = nullptr;

    int ret = yyparse(ast);
    fclose(yyin);

    if (ret != 0) {
        cerr << "parse failed\n";
        return 1;
    }

    if (!ast) {
        cerr << "empty AST\n";
        return 1;
    }

    ast->Dump(0);

    return 0;
}