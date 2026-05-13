#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

// AST 基类: 所有语法树节点都能输出 Koopa IR 文本.
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  // 输出 Koopa IR 文本
  virtual void DumpKoopa(std::ostream &out) const = 0;
};

// 表达式基类: 支持生成表达式的 Koopa IR 值.
class ExprAST : public BaseAST {
 public:
  // 生成表达式值, 返回 Koopa IR 的值名或立即数
  virtual std::string DumpKoopaValue(std::ostream &out) const = 0;
  // 计算常量表达式的值
  virtual int EvalConst() const = 0;
  void DumpKoopa(std::ostream &out) const override { (void)DumpKoopaValue(out); }
};

// 编译单元: 当前仅包含一个函数定义.
class CompUnitAST : public BaseAST {
 public:
  // 顶层只包含一个函数定义
  std::unique_ptr<BaseAST> func_def;

  void DumpKoopa(std::ostream &out) const override;
};

// 函数返回类型: 目前仅支持 int.
class FuncTypeAST : public BaseAST {
 public:
  // 函数返回类型名, 当前仅支持 "int"
  std::string name;

  void DumpKoopa(std::ostream &out) const override;
};

// 语句块: 目前只包含单条语句.
class BlockAST : public BaseAST {
 public:
  // 块内语句或声明列表
  std::vector<std::unique_ptr<BaseAST>> items;

  void DumpKoopa(std::ostream &out) const override;
};

// return 语句.
class ReturnStmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> ret_exp;

  void DumpKoopa(std::ostream &out) const override;
};

// 赋值语句.
class AssignStmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> value;

  void DumpKoopa(std::ostream &out) const override;
};

// 函数定义: 定义函数名、返回类型与函数体.
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

// 整数字面量表达式.
class NumberAST : public ExprAST {
 public:
  int value = 0;

  std::string DumpKoopaValue(std::ostream &out) const override;
  int EvalConst() const override;
};

// 基本表达式: 括号表达式或数字.
class PrimaryExpAST : public ExprAST {
 public:
  // (Exp) 或 Number
  std::unique_ptr<BaseAST> inner;

  std::string DumpKoopaValue(std::ostream &out) const override;
  int EvalConst() const override;
};

// 左值表达式: 标识符
class LValAST : public ExprAST {
 public:
  std::string ident;

  std::string DumpKoopaValue(std::ostream &out) const override;
  int EvalConst() const override;
};

// 一元表达式: +, -, ! 以及其操作数.
class UnaryExpAST : public ExprAST {
 public:
  char op = 0;
  std::unique_ptr<BaseAST> operand;

  std::string DumpKoopaValue(std::ostream &out) const override;
  int EvalConst() const override;
};

// 二元运算符枚举: 覆盖算术、比较与逻辑运算.
enum class BinaryOp {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Lt,
  Gt,
  Le,
  Ge,
  Eq,
  Ne,
  And,
  Or,
};

// 二元表达式: 左右操作数加运算符.
class BinaryExpAST : public ExprAST {
 public:
  BinaryOp op = BinaryOp::Add;
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> rhs;

  std::string DumpKoopaValue(std::ostream &out) const override;
  int EvalConst() const override;
};

// 常量定义
class ConstDefAST : public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> init;

  void DumpKoopa(std::ostream &out) const override;
};

// 常量声明
class ConstDeclAST : public BaseAST {
 public:
  std::vector<std::unique_ptr<BaseAST>> defs;

  void DumpKoopa(std::ostream &out) const override;
};

// 变量定义
class VarDefAST : public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> init;

  void DumpKoopa(std::ostream &out) const override;
};

// 变量声明
class VarDeclAST : public BaseAST {
 public:
  std::vector<std::unique_ptr<BaseAST>> defs;

  void DumpKoopa(std::ostream &out) const override;
};
