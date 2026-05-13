#include "ast.h"

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

namespace {
int g_temp_id = 0;
int g_alloc_id = 0;

struct SymbolInfo {
  enum class Kind { Const, Var };
  Kind kind = Kind::Const;
  int const_value = 0;
  std::string alloc_name;
};

std::vector<std::unordered_map<std::string, SymbolInfo>> g_scopes;

std::string NextTemp() {
  // 生成新的 SSA 临时值名
  return "%" + std::to_string(g_temp_id++);
}

std::string NextAllocName(const std::string &ident) {
  // 生成唯一的 alloc 名称, 避免同名变量冲突
  return "@" + ident + "_" + std::to_string(g_alloc_id++);
}

void EnterScope() {
  // 进入新作用域
  g_scopes.emplace_back();
}

void ExitScope() {
  // 离开作用域
  assert(!g_scopes.empty());
  g_scopes.pop_back();
}

SymbolInfo *LookupSymbol(const std::string &name) {
  // 从内到外查找符号
  for (auto it = g_scopes.rbegin(); it != g_scopes.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end()) {
      return &found->second;
    }
  }
  return nullptr;
}

void InsertSymbol(const std::string &name, const SymbolInfo &info) {
  // 当前作用域内禁止重名
  assert(!g_scopes.empty());
  auto &scope = g_scopes.back();
  assert(scope.find(name) == scope.end());
  scope.emplace(name, info);
}

std::string EmitBinary(std::ostream &out, const std::string &op, const std::string &lhs,
                       const std::string &rhs) {
  // 输出一条二元指令并返回结果值名
  std::string result = NextTemp();
  out << "  " << result << " = " << op << " " << lhs << ", " << rhs << "\n";
  return result;
}

std::string EmitLoad(std::ostream &out, const std::string &addr) {
  // 输出一条 load 指令
  std::string result = NextTemp();
  out << "  " << result << " = load " << addr << "\n";
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
  // 输出块内所有语句或声明
  EnterScope();
  for (const auto &item : items) {
    item->DumpKoopa(out);
  }
  ExitScope();
}

void ReturnStmtAST::DumpKoopa(std::ostream &out) const {
  // 输出 return 指令及其返回值
  // 生成 return 指令
  auto *expr = dynamic_cast<ExprAST *>(ret_exp.get());
  assert(expr != nullptr);
  std::string value = expr->DumpKoopaValue(out);
  out << "  ret " << value << "\n";
}

void AssignStmtAST::DumpKoopa(std::ostream &out) const {
  // 赋值语句: 计算右值并写回变量
  auto *lhs = dynamic_cast<LValAST *>(lval.get());
  auto *rhs = dynamic_cast<ExprAST *>(value.get());
  assert(lhs != nullptr && rhs != nullptr);
  SymbolInfo *info = LookupSymbol(lhs->ident);
  assert(info != nullptr && info->kind == SymbolInfo::Kind::Var);
  std::string rhs_value = rhs->DumpKoopaValue(out);
  out << "  store " << rhs_value << ", " << info->alloc_name << "\n";
}

void FuncDefAST::DumpKoopa(std::ostream &out) const {
  // 输出函数定义与入口基本块
  // 输出函数头和入口基本块
  out << "fun @" << ident << "(): i32 {\n";
  out << "%entry:\n";
  // 每个函数从 0 开始编号临时变量
  g_temp_id = 0;
  g_alloc_id = 0;
  g_scopes.clear();
  EnterScope();
  // 输出函数体
  block->DumpKoopa(out);
  ExitScope();
  // 结束函数
  out << "}\n";
}

std::string NumberAST::DumpKoopaValue(std::ostream &out) const {
  // 数字常量直接作为立即数返回
  (void)out;
  return std::to_string(value);
}

int NumberAST::EvalConst() const {
  return value;
}

std::string PrimaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 直接转发到内部表达式
  auto *expr = dynamic_cast<ExprAST *>(inner.get());
  assert(expr != nullptr);
  return expr->DumpKoopaValue(out);
}

int PrimaryExpAST::EvalConst() const {
  auto *expr = dynamic_cast<ExprAST *>(inner.get());
  assert(expr != nullptr);
  return expr->EvalConst();
}

std::string LValAST::DumpKoopaValue(std::ostream &out) const {
  // 读取变量或常量
  SymbolInfo *info = LookupSymbol(ident);
  assert(info != nullptr);
  if (info->kind == SymbolInfo::Kind::Const) {
    return std::to_string(info->const_value);
  }
  return EmitLoad(out, info->alloc_name);
}

int LValAST::EvalConst() const {
  SymbolInfo *info = LookupSymbol(ident);
  assert(info != nullptr && info->kind == SymbolInfo::Kind::Const);
  return info->const_value;
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

int UnaryExpAST::EvalConst() const {
  auto *expr = dynamic_cast<ExprAST *>(operand.get());
  assert(expr != nullptr);
  int value = expr->EvalConst();
  if (op == '+') {
    return value;
  }
  if (op == '-') {
    return -value;
  }
  if (op == '!') {
    return value == 0 ? 1 : 0;
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

int BinaryExpAST::EvalConst() const {
  auto *left = dynamic_cast<ExprAST *>(lhs.get());
  auto *right = dynamic_cast<ExprAST *>(rhs.get());
  assert(left != nullptr && right != nullptr);
  int lhs_value = left->EvalConst();
  int rhs_value = right->EvalConst();
  switch (op) {
    case BinaryOp::Add:
      return lhs_value + rhs_value;
    case BinaryOp::Sub:
      return lhs_value - rhs_value;
    case BinaryOp::Mul:
      return lhs_value * rhs_value;
    case BinaryOp::Div:
      return lhs_value / rhs_value;
    case BinaryOp::Mod:
      return lhs_value % rhs_value;
    case BinaryOp::Lt:
      return lhs_value < rhs_value ? 1 : 0;
    case BinaryOp::Gt:
      return lhs_value > rhs_value ? 1 : 0;
    case BinaryOp::Le:
      return lhs_value <= rhs_value ? 1 : 0;
    case BinaryOp::Ge:
      return lhs_value >= rhs_value ? 1 : 0;
    case BinaryOp::Eq:
      return lhs_value == rhs_value ? 1 : 0;
    case BinaryOp::Ne:
      return lhs_value != rhs_value ? 1 : 0;
    case BinaryOp::And:
      return (lhs_value != 0 && rhs_value != 0) ? 1 : 0;
    case BinaryOp::Or:
      return (lhs_value != 0 || rhs_value != 0) ? 1 : 0;
  }
  assert(false);
  return lhs_value;
}

void ConstDefAST::DumpKoopa(std::ostream &out) const {
  // 常量定义只需要在编译期求值
  (void)out;
  auto *expr = dynamic_cast<ExprAST *>(init.get());
  assert(expr != nullptr);
  SymbolInfo info;
  info.kind = SymbolInfo::Kind::Const;
  info.const_value = expr->EvalConst();
  InsertSymbol(ident, info);
}

void ConstDeclAST::DumpKoopa(std::ostream &out) const {
  // 遍历常量定义
  for (const auto &def : defs) {
    def->DumpKoopa(out);
  }
}

void VarDefAST::DumpKoopa(std::ostream &out) const {
  // 变量定义: 生成 alloc, 可选初始化
  SymbolInfo info;
  info.kind = SymbolInfo::Kind::Var;
  info.alloc_name = NextAllocName(ident);
  out << "  " << info.alloc_name << " = alloc i32\n";
  InsertSymbol(ident, info);
  if (init) {
    auto *expr = dynamic_cast<ExprAST *>(init.get());
    assert(expr != nullptr);
    std::string init_value = expr->DumpKoopaValue(out);
    out << "  store " << init_value << ", " << info.alloc_name << "\n";
  }
}

void VarDeclAST::DumpKoopa(std::ostream &out) const {
  // 遍历变量定义
  for (const auto &def : defs) {
    def->DumpKoopa(out);
  }
}
