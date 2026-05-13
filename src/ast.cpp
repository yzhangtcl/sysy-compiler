#include "ast.h"

#include <utility>

void CompUnitAST::DumpKoopa(std::ostream &out) const {
  func_def->DumpKoopa(out);
}

void FuncTypeAST::DumpKoopa(std::ostream &out) const {
  (void)out;
}

void BlockAST::DumpKoopa(std::ostream &out) const {
  stmt->DumpKoopa(out);
}

void StmtAST::DumpKoopa(std::ostream &out) const {
  out << "  ret " << ret_value << "\n";
}

void FuncDefAST::DumpKoopa(std::ostream &out) const {
  out << "fun @" << ident << "(): i32 {\n";
  out << "%entry:\n";
  block->DumpKoopa(out);
  out << "}\n";
}
