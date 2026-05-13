#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "ast.h"
#include "koopa.h"
#include "visit.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  // mode: -koopa 或 -riscv
  auto mode = argv[1];
  // input: 源文件路径
  auto input = argv[2];
  // output: 输出文件路径
  auto output = argv[4];

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  // ast: 语法树根节点
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // 统一写入输出文件
  ofstream out(output);
  assert(out.is_open());
  if (string(mode) == "-koopa") {
    // 直接输出 Koopa IR 文本
    ast->DumpKoopa(out);
    return 0;
  }

  if (string(mode) == "-riscv") {
    // 先生成 Koopa IR 文本, 再交给 libkoopa 解析
    std::ostringstream koopa_stream;
    ast->DumpKoopa(koopa_stream);
    const std::string &ir = koopa_stream.str();

    // program: 解析得到的 Koopa IR
    koopa_program_t program;
    koopa_error_code_t parse_ret = koopa_parse_from_string(ir.c_str(), &program);
    assert(parse_ret == KOOPA_EC_SUCCESS);
    // builder/raw: 内存形式 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    // 生成 RISC-V 汇编
    AsmGenerator generator;
    generator.Generate(raw, out);

    koopa_delete_raw_program_builder(builder);
    return 0;
  }
  return 0;
}