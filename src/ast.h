#pragma once

#include <memory>
#include <ostream>
#include <string>

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  // 输出 Koopa IR 文本
  virtual void DumpKoopa(std::ostream &out) const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  // 顶层只包含一个函数定义
  std::unique_ptr<BaseAST> func_def;

  void DumpKoopa(std::ostream &out) const override;
};

class FuncTypeAST : public BaseAST {
 public:
  // 函数返回类型名, 当前仅支持 "int"
  std::string name;

  void DumpKoopa(std::ostream &out) const override;
};

class BlockAST : public BaseAST {
 public:
  // 块内唯一的语句
  std::unique_ptr<BaseAST> stmt;

  void DumpKoopa(std::ostream &out) const override;
};

class StmtAST : public BaseAST {
 public:
  // return 的立即数
  int ret_value = 0;

  void DumpKoopa(std::ostream &out) const override;
};

class FuncDefAST : public BaseAST {
 public:
  // 返回类型
  std::unique_ptr<BaseAST> func_type;
  // 函数名, 本章固定为 main
  std::string ident;
  // 函数体
  std::unique_ptr<BaseAST> block;

  void DumpKoopa(std::ostream &out) const override;
};
