#include "ast.h"

#include <utility>

void CompUnitAST::DumpKoopa(std::ostream &out) const {
  // 入口直接转发到函数定义
  func_def->DumpKoopa(out);
}

void FuncTypeAST::DumpKoopa(std::ostream &out) const {
  // 类型信息在当前阶段不直接输出
  (void)out;
}

void BlockAST::DumpKoopa(std::ostream &out) const {
  // 输出块内语句
  stmt->DumpKoopa(out);
}

void StmtAST::DumpKoopa(std::ostream &out) const {
  // 生成 return 指令
  out << "  ret " << ret_value << "\n";
}

void FuncDefAST::DumpKoopa(std::ostream &out) const {
  // 输出函数头和入口基本块
  out << "fun @" << ident << "(): i32 {\n";
  out << "%entry:\n";
  // 输出函数体
  block->DumpKoopa(out);
  // 结束函数
  out << "}\n";
}
