#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <cstring>
#include "AST.h"
#include "koopa.h"
#include "visit.h"
#include "utils.h"
#include "Symbol.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

extern RiscvString rvs;
extern KoopaString ks;

int main(int argc, const char *argv[]) {
    assert(argc == 5);
    string mode = argv[1];
    string input = argv[2];
    string output = argv[4];

    yyin = fopen(input.c_str(), "r");
    assert(yyin);

    ofstream fout(output);
    assert(fout.is_open());

    ks.clear();
    rvs.clear();

    unique_ptr<BaseAST> base_ast;
    int ret = yyparse(base_ast);
    assert(ret == 0);

    auto ast = unique_ptr<CompUnitAST>((CompUnitAST *)base_ast.release());
    ast->Dump();

    if (mode == "-koopa") {
        fout << ks.c_str();
        fout.close();
        return 0;
    }

    if (mode != "-riscv") {
        cerr << "Unknown mode: " << mode << endl;
        return 1;
    }

    // -riscv 模式
    koopa_program_t program;
    auto err = koopa_parse_from_string(ks.c_str(), &program);
    assert(err == KOOPA_EC_SUCCESS);

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    Visit(raw);

    fout << rvs.c_str();
    fout.close();

    koopa_delete_raw_program_builder(builder);
    return 0;
}