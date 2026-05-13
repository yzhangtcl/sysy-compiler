#pragma once

#include <memory>
#include <ostream>
#include <string>

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void DumpKoopa(std::ostream &out) const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  void DumpKoopa(std::ostream &out) const override {
    func_def->DumpKoopa(out);
  }
};

class FuncTypeAST : public BaseAST {
 public:
  std::string name;

  void DumpKoopa(std::ostream &out) const override {
    (void)out;
  }
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;

  void DumpKoopa(std::ostream &out) const override {
    stmt->DumpKoopa(out);
  }
};

class StmtAST : public BaseAST {
 public:
  int ret_value = 0;

  void DumpKoopa(std::ostream &out) const override {
    out << "  ret " << ret_value << "\n";
  }
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void DumpKoopa(std::ostream &out) const override {
    out << "fun @" << ident << "(): i32 {\n";
    out << "%entry:\n";
    block->DumpKoopa(out);
    out << "}\n";
  }
};
