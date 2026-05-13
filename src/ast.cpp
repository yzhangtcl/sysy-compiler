#include "ast.h"

#include <cassert>
#include <string>
#include <utility>

namespace {
int g_temp_id = 0;

std::string NextTemp() {
  // 生成新的 SSA 临时值名
  return "%" + std::to_string(g_temp_id++);
}

std::string EmitBinary(std::ostream &out, const std::string &op, const std::string &lhs,
                       const std::string &rhs) {
  // 输出一条二元指令并返回结果值名
  std::string result = NextTemp();
  out << "  " << result << " = " << op << " " << lhs << ", " << rhs << "\n";
  return result;
}

std::string EmitNotZero(std::ostream &out, const std::string &value) {
  // 生成非零判断: value != 0
  return EmitBinary(out, "ne", value, "0");
}
}  // namespace

void CompUnitAST::DumpKoopa(std::ostream &out) const {
  // 顶层只包含一个函数定义
  // 入口直接转发到函数定义
  func_def->DumpKoopa(out);
}

void FuncTypeAST::DumpKoopa(std::ostream &out) const {
  // 当前阶段不输出类型, 保留接口以兼容后续扩展
  // 类型信息在当前阶段不直接输出
  (void)out;
}

void BlockAST::DumpKoopa(std::ostream &out) const {
  // 输出块内唯一的语句
  // 输出块内语句
  stmt->DumpKoopa(out);
}

void StmtAST::DumpKoopa(std::ostream &out) const {
  // 输出 return 指令及其返回值
  // 生成 return 指令
  auto *expr = dynamic_cast<ExprAST *>(ret_exp.get());
  assert(expr != nullptr);
  std::string value = expr->DumpKoopaValue(out);
  out << "  ret " << value << "\n";
}

void FuncDefAST::DumpKoopa(std::ostream &out) const {
  // 输出函数定义与入口基本块
  // 输出函数头和入口基本块
  out << "fun @" << ident << "(): i32 {\n";
  out << "%entry:\n";
  // 每个函数从 0 开始编号临时变量
  g_temp_id = 0;
  // 输出函数体
  block->DumpKoopa(out);
  // 结束函数
  out << "}\n";
}

std::string NumberAST::DumpKoopaValue(std::ostream &out) const {
  // 数字常量直接作为立即数返回
  (void)out;
  return std::to_string(value);
}

std::string PrimaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 直接转发到内部表达式
  auto *expr = dynamic_cast<ExprAST *>(inner.get());
  assert(expr != nullptr);
  return expr->DumpKoopaValue(out);
}

std::string UnaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 一元表达式: + 原值, - 变号, ! 逻辑非
  auto *expr = dynamic_cast<ExprAST *>(operand.get());
  assert(expr != nullptr);
  std::string value = expr->DumpKoopaValue(out);
  if (op == '+') {
    return value;
  }
  if (op == '-') {
    return EmitBinary(out, "sub", "0", value);
  }
  if (op == '!') {
    return EmitBinary(out, "eq", value, "0");
  }
  assert(false);
  return value;
}

std::string BinaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 二元表达式: 先生成左右值, 再输出对应二元指令
  auto *left = dynamic_cast<ExprAST *>(lhs.get());
  auto *right = dynamic_cast<ExprAST *>(rhs.get());
  assert(left != nullptr && right != nullptr);
  std::string lhs_value = left->DumpKoopaValue(out);
  std::string rhs_value = right->DumpKoopaValue(out);

  switch (op) {
    case BinaryOp::Add:
      return EmitBinary(out, "add", lhs_value, rhs_value);
    case BinaryOp::Sub:
      return EmitBinary(out, "sub", lhs_value, rhs_value);
    case BinaryOp::Mul:
      return EmitBinary(out, "mul", lhs_value, rhs_value);
    case BinaryOp::Div:
      return EmitBinary(out, "div", lhs_value, rhs_value);
    case BinaryOp::Mod:
      return EmitBinary(out, "mod", lhs_value, rhs_value);
    case BinaryOp::Lt:
      return EmitBinary(out, "lt", lhs_value, rhs_value);
    case BinaryOp::Gt:
      return EmitBinary(out, "gt", lhs_value, rhs_value);
    case BinaryOp::Le:
      return EmitBinary(out, "le", lhs_value, rhs_value);
    case BinaryOp::Ge:
      return EmitBinary(out, "ge", lhs_value, rhs_value);
    case BinaryOp::Eq:
      return EmitBinary(out, "eq", lhs_value, rhs_value);
    case BinaryOp::Ne:
      return EmitBinary(out, "ne", lhs_value, rhs_value);
    case BinaryOp::And: {
      // 逻辑与: 先归一化为 0/1, 再按位与并归一化
      std::string lhs_bool = EmitNotZero(out, lhs_value);
      std::string rhs_bool = EmitNotZero(out, rhs_value);
      std::string and_value = EmitBinary(out, "and", lhs_bool, rhs_bool);
      return EmitNotZero(out, and_value);
    }
    case BinaryOp::Or: {
      // 逻辑或: 先归一化为 0/1, 再按位或并归一化
      std::string lhs_bool = EmitNotZero(out, lhs_value);
      std::string rhs_bool = EmitNotZero(out, rhs_value);
      std::string or_value = EmitBinary(out, "or", lhs_bool, rhs_bool);
      return EmitNotZero(out, or_value);
    }
  }
  assert(false);
  return lhs_value;
}
