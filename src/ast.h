#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

// 值类型: 当前支持 int, float 与 void (仅用于函数返回类型).
enum class ValueType { Int, Float, Void };

// 表达式生成的值与类型.
struct ValueResult {
  std::string name;
  ValueType type = ValueType::Int;
};

// 常量表达式求值结果.
struct ConstValue {
  ValueType type = ValueType::Int;
  int32_t int_value = 0;
  float float_value = 0.0f;
};

// 类型信息表 (供后端查询).
ValueType LookupValueType(const std::string &name);
ValueType LookupFunctionReturnType(const std::string &name);
void RegisterValueType(const std::string &name, ValueType type);
void RegisterFunctionReturnType(const std::string &name, ValueType type);
void ResetValueTypeTable();

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
  virtual ValueResult DumpKoopaValue(std::ostream &out) const = 0;
  // 计算常量表达式的值
  virtual ConstValue EvalConst() const = 0;
  void DumpKoopa(std::ostream &out) const override { (void)DumpKoopaValue(out); }
};

// 编译单元: 包含全局声明和函数定义列表.
class CompUnitAST : public BaseAST {
 public:
  // 顶层可包含多个全局声明 (Decl) 或函数定义 (FuncDef)
  std::vector<std::unique_ptr<BaseAST>> items;

  void DumpKoopa(std::ostream &out) const override;
};

// 函数返回类型: 支持 "int", "float", "void".
class FuncTypeAST : public BaseAST {
 public:
  // 函数返回类型名
  std::string name;
  ValueType value_type = ValueType::Int;

  void DumpKoopa(std::ostream &out) const override;
};

// 语句块: 包含多条语句或声明.
class BlockAST : public BaseAST {
 public:
  // 块内语句或声明列表
  std::vector<std::unique_ptr<BaseAST>> items;

  void DumpKoopa(std::ostream &out) const override;
};

// return 语句: 可选的返回值, 用于 void 函数.
class ReturnStmtAST : public BaseAST {
 public:
  // 可选的返回表达式; 为空表示 void 返回
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

// 表达式语句: 可为空 (单独的分号).
class ExprStmtAST : public BaseAST {
 public:
  // 可选表达式, 为空表示仅包含 ';'
  std::unique_ptr<BaseAST> expr;

  void DumpKoopa(std::ostream &out) const override;
};

// if 语句: 支持可选 else 分支.
class IfStmtAST : public BaseAST {
 public:
  // 条件表达式
  std::unique_ptr<BaseAST> cond;
  // if 分支语句
  std::unique_ptr<BaseAST> then_stmt;
  // 可选 else 分支语句
  std::unique_ptr<BaseAST> else_stmt;

  void DumpKoopa(std::ostream &out) const override;
};

// while 语句: 条件表达式 + 循环体.
class WhileStmtAST : public BaseAST {
 public:
  // 循环条件表达式
  std::unique_ptr<BaseAST> cond;
  // 循环体语句
  std::unique_ptr<BaseAST> body;

  void DumpKoopa(std::ostream &out) const override;
};

// break 语句.
class BreakStmtAST : public BaseAST {
 public:
  void DumpKoopa(std::ostream &out) const override;
};

// continue 语句.
class ContinueStmtAST : public BaseAST {
 public:
  void DumpKoopa(std::ostream &out) const override;
};

// 函数定义: 定义函数名、参数、返回类型与函数体.
class FuncDefAST : public BaseAST {
 public:
  // 返回类型
  std::unique_ptr<BaseAST> func_type;
  // 函数名
  std::string ident;
  // 形式参数列表 (FuncFParamAST)
  std::vector<std::unique_ptr<BaseAST>> params;
  // 函数体
  std::unique_ptr<BaseAST> block;

  void DumpKoopa(std::ostream &out) const override;
};

// 函数形式参数: 类型 + 标识符.
class FuncFParamAST : public BaseAST {
 public:
  // 参数类型, 当前支持 int / float
  ValueType value_type = ValueType::Int;
  // 参数名
  std::string ident;

  void DumpKoopa(std::ostream &out) const override;
};

// 整数字面量表达式.
class NumberAST : public ExprAST {
 public:
  int value = 0;

  ValueResult DumpKoopaValue(std::ostream &out) const override;
  ConstValue EvalConst() const override;
};

// 浮点字面量表达式.
class FloatNumberAST : public ExprAST {
 public:
  float value = 0.0f;

  ValueResult DumpKoopaValue(std::ostream &out) const override;
  ConstValue EvalConst() const override;
};

// 基本表达式: 括号表达式或数字.
class PrimaryExpAST : public ExprAST {
 public:
  // (Exp) 或 Number
  std::unique_ptr<BaseAST> inner;

  ValueResult DumpKoopaValue(std::ostream &out) const override;
  ConstValue EvalConst() const override;
};

// 左值表达式: 标识符
class LValAST : public ExprAST {
 public:
  std::string ident;

  ValueResult DumpKoopaValue(std::ostream &out) const override;
  ConstValue EvalConst() const override;
};

// 一元表达式: +, -, !, 函数调用以及其操作数.
class UnaryExpAST : public ExprAST {
 public:
  char op = 0;
  std::unique_ptr<BaseAST> operand;
  // 函数调用: op==0 且 call_ident 非空表示 func(args)
  std::string call_ident;
  std::vector<std::unique_ptr<BaseAST>> call_args;

  ValueResult DumpKoopaValue(std::ostream &out) const override;
  ConstValue EvalConst() const override;
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

  ValueResult DumpKoopaValue(std::ostream &out) const override;
  ConstValue EvalConst() const override;
};

// 常量定义
class ConstDefAST : public BaseAST {
 public:
  std::string ident;
  ValueType value_type = ValueType::Int;
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
  ValueType value_type = ValueType::Int;
  std::unique_ptr<BaseAST> init;

  void DumpKoopa(std::ostream &out) const override;
};

// 变量声明
class VarDeclAST : public BaseAST {
 public:
  std::vector<std::unique_ptr<BaseAST>> defs;

  void DumpKoopa(std::ostream &out) const override;
};
