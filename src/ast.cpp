#include "ast.h"

#include <cassert>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

namespace {
int g_temp_id = 0;
int g_alloc_id = 0;
int g_block_id = 0;
bool g_block_terminated = false;
std::unordered_map<std::string, ValueType> g_value_types;
std::unordered_map<std::string, ValueType> g_function_return_types;

struct SymbolInfo {
  enum class Kind { Const, Var };
  Kind kind = Kind::Const;
  ValueType value_type = ValueType::Int;
  ConstValue const_value;
  std::string alloc_name;
};

std::vector<std::unordered_map<std::string, SymbolInfo>> g_scopes;

std::string NextTemp(ValueType type) {
  // 生成新的 SSA 临时值名, 前缀区分 int/float
  const char *prefix = type == ValueType::Float ? "%f" : "%t";
  return std::string(prefix) + std::to_string(g_temp_id++);
}

std::string NextAllocName(const std::string &ident) {
  // 生成唯一的 alloc 名称, 避免同名变量冲突
  return "@" + ident + "_" + std::to_string(g_alloc_id++);
}

std::string NextBlockLabel(const std::string &prefix) {
  // 生成唯一的基本块标签
  return "%" + prefix + "_" + std::to_string(g_block_id++);
}

int32_t FloatToBits(float value) {
  // 将 float 视为 32-bit bit pattern
  static_assert(sizeof(float) == sizeof(uint32_t));
  uint32_t bits = 0;
  std::memcpy(&bits, &value, sizeof(bits));
  return static_cast<int32_t>(bits);
}

float ToFloat(const ConstValue &value) {
  return value.type == ValueType::Float ? value.float_value
                                        : static_cast<float>(value.int_value);
}

int32_t ToInt(const ConstValue &value) {
  return value.type == ValueType::Int ? value.int_value
                                      : static_cast<int32_t>(value.float_value);
}

ConstValue CastConstValue(const ConstValue &value, ValueType target) {
  ConstValue result;
  result.type = target;
  if (target == ValueType::Float) {
    result.float_value = ToFloat(value);
    result.int_value = static_cast<int32_t>(result.float_value);
    return result;
  }
  result.int_value = ToInt(value);
  result.float_value = static_cast<float>(result.int_value);
  return result;
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

ValueResult EmitBinary(std::ostream &out, const std::string &op, const std::string &lhs,
                       const std::string &rhs, ValueType result_type) {
  // 输出一条二元指令并返回结果
  std::string result = NextTemp(result_type);
  out << "  " << result << " = " << op << " " << lhs << ", " << rhs << "\n";
  RegisterValueType(result, result_type);
  return {result, result_type};
}

ValueResult EmitLoad(std::ostream &out, const std::string &addr, ValueType result_type) {
  // 输出一条 load 指令
  std::string result = NextTemp(result_type);
  out << "  " << result << " = load " << addr << "\n";
  RegisterValueType(result, result_type);
  return {result, result_type};
}

ValueResult EmitNotZero(std::ostream &out, const ValueResult &value) {
  // 生成非零判断: value != 0
  return EmitBinary(out, "ne", value.name, "0", ValueType::Int);
}

void EmitLabel(std::ostream &out, const std::string &label) {
  // 输出基本块标签并重置终结状态
  out << label << ":\n";
  g_block_terminated = false;
}

void EmitJump(std::ostream &out, const std::string &label) {
  // 输出无条件跳转, 终结当前基本块
  out << "  jump " << label << "\n";
  g_block_terminated = true;
}

void EmitBranch(std::ostream &out, const std::string &cond, const std::string &true_label,
                const std::string &false_label) {
  // 输出条件分支, 终结当前基本块
  out << "  br " << cond << ", " << true_label << ", " << false_label << "\n";
  g_block_terminated = true;
}

bool IsBlockTerminated() {
  return g_block_terminated;
}
}  // namespace

ValueType LookupValueType(const std::string &name) {
  auto it = g_value_types.find(name);
  if (it != g_value_types.end()) {
    return it->second;
  }
  return ValueType::Int;
}

ValueType LookupFunctionReturnType(const std::string &name) {
  auto it = g_function_return_types.find(name);
  if (it != g_function_return_types.end()) {
    return it->second;
  }
  return ValueType::Int;
}

void RegisterValueType(const std::string &name, ValueType type) {
  g_value_types[name] = type;
}

void RegisterFunctionReturnType(const std::string &name, ValueType type) {
  g_function_return_types[name] = type;
}

void ResetValueTypeTable() {
  g_value_types.clear();
}

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
    if (IsBlockTerminated()) {
      // 当前基本块已结束, 剩余语句不可达
      break;
    }
    item->DumpKoopa(out);
  }
  ExitScope();
}

void ReturnStmtAST::DumpKoopa(std::ostream &out) const {
  // 输出 return 指令及其返回值
  // 生成 return 指令
  auto *expr = dynamic_cast<ExprAST *>(ret_exp.get());
  assert(expr != nullptr);
  ValueResult value = expr->DumpKoopaValue(out);
  out << "  ret " << value.name << "\n";
  g_block_terminated = true;
}

void AssignStmtAST::DumpKoopa(std::ostream &out) const {
  // 赋值语句: 计算右值并写回变量
  auto *lhs = dynamic_cast<LValAST *>(lval.get());
  auto *rhs = dynamic_cast<ExprAST *>(value.get());
  assert(lhs != nullptr && rhs != nullptr);
  SymbolInfo *info = LookupSymbol(lhs->ident);
  assert(info != nullptr && info->kind == SymbolInfo::Kind::Var);
  ValueResult rhs_value = rhs->DumpKoopaValue(out);
  out << "  store " << rhs_value.name << ", " << info->alloc_name << "\n";
}

void ExprStmtAST::DumpKoopa(std::ostream &out) const {
  // 表达式语句需要保留副作用, 空语句不输出任何 IR
  if (!expr) {
    return;
  }
  auto *stmt_expr = dynamic_cast<ExprAST *>(expr.get());
  assert(stmt_expr != nullptr);
  (void)stmt_expr->DumpKoopaValue(out);
}

void IfStmtAST::DumpKoopa(std::ostream &out) const {
  // if 语句: 生成条件分支与汇合基本块
  auto *cond_expr = dynamic_cast<ExprAST *>(cond.get());
  assert(cond_expr != nullptr);

  ValueResult cond_value = cond_expr->DumpKoopaValue(out);
  ValueResult cond_bool = EmitNotZero(out, cond_value);

  std::string then_label = NextBlockLabel("if_then");
  std::string end_label = NextBlockLabel("if_end");
  std::string else_label = else_stmt ? NextBlockLabel("if_else") : end_label;

  EmitBranch(out, cond_bool.name, then_label, else_label);

  EmitLabel(out, then_label);
  then_stmt->DumpKoopa(out);
  if (!IsBlockTerminated()) {
    EmitJump(out, end_label);
  }

  if (else_stmt) {
    EmitLabel(out, else_label);
    else_stmt->DumpKoopa(out);
    if (!IsBlockTerminated()) {
      EmitJump(out, end_label);
    }
  }

  EmitLabel(out, end_label);
}

void FuncDefAST::DumpKoopa(std::ostream &out) const {
  // 输出函数定义与入口基本块
  // 输出函数头和入口基本块
  out << "fun @" << ident << "(): i32 {\n";
  // 每个函数从 0 开始编号临时变量
  g_temp_id = 0;
  g_alloc_id = 0;
  g_block_id = 0;
  g_block_terminated = false;
  g_scopes.clear();
  ResetValueTypeTable();
  auto *type_ast = dynamic_cast<FuncTypeAST *>(func_type.get());
  RegisterFunctionReturnType(ident, type_ast ? type_ast->value_type : ValueType::Int);
  EnterScope();
  EmitLabel(out, "%entry");
  // 输出函数体
  block->DumpKoopa(out);
  ExitScope();
  // 结束函数
  out << "}\n";
}

ValueResult NumberAST::DumpKoopaValue(std::ostream &out) const {
  // 数字常量直接作为立即数返回
  (void)out;
  return {std::to_string(value), ValueType::Int};
}

ConstValue NumberAST::EvalConst() const {
  ConstValue result;
  result.type = ValueType::Int;
  result.int_value = value;
  result.float_value = static_cast<float>(value);
  return result;
}

ValueResult FloatNumberAST::DumpKoopaValue(std::ostream &out) const {
  // 浮点常量以 bit pattern 形式落在 IR 中
  int32_t bits = FloatToBits(value);
  return EmitBinary(out, "add", "0", std::to_string(bits), ValueType::Float);
}

ConstValue FloatNumberAST::EvalConst() const {
  ConstValue result;
  result.type = ValueType::Float;
  result.float_value = value;
  result.int_value = static_cast<int32_t>(value);
  return result;
}

ValueResult PrimaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 直接转发到内部表达式
  auto *expr = dynamic_cast<ExprAST *>(inner.get());
  assert(expr != nullptr);
  return expr->DumpKoopaValue(out);
}

ConstValue PrimaryExpAST::EvalConst() const {
  auto *expr = dynamic_cast<ExprAST *>(inner.get());
  assert(expr != nullptr);
  return expr->EvalConst();
}

ValueResult LValAST::DumpKoopaValue(std::ostream &out) const {
  // 读取变量或常量
  SymbolInfo *info = LookupSymbol(ident);
  assert(info != nullptr);
  if (info->kind == SymbolInfo::Kind::Const) {
    if (info->value_type == ValueType::Int) {
      return {std::to_string(info->const_value.int_value), ValueType::Int};
    }
    int32_t bits = FloatToBits(info->const_value.float_value);
    return EmitBinary(out, "add", "0", std::to_string(bits), ValueType::Float);
  }
  return EmitLoad(out, info->alloc_name, info->value_type);
}

ConstValue LValAST::EvalConst() const {
  SymbolInfo *info = LookupSymbol(ident);
  assert(info != nullptr && info->kind == SymbolInfo::Kind::Const);
  return info->const_value;
}

ValueResult UnaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 一元表达式: + 原值, - 变号, ! 逻辑非
  auto *expr = dynamic_cast<ExprAST *>(operand.get());
  assert(expr != nullptr);
  ValueResult value = expr->DumpKoopaValue(out);
  if (op == '+') {
    return value;
  }
  if (op == '-') {
    return EmitBinary(out, "sub", "0", value.name, value.type);
  }
  if (op == '!') {
    return EmitBinary(out, "eq", value.name, "0", ValueType::Int);
  }
  assert(false);
  return value;
}

ConstValue UnaryExpAST::EvalConst() const {
  auto *expr = dynamic_cast<ExprAST *>(operand.get());
  assert(expr != nullptr);
  ConstValue value = expr->EvalConst();
  if (op == '+') {
    return value;
  }
  if (op == '-') {
    if (value.type == ValueType::Float) {
      value.float_value = -value.float_value;
      value.int_value = static_cast<int32_t>(value.float_value);
      return value;
    }
    value.int_value = -value.int_value;
    value.float_value = static_cast<float>(value.int_value);
    return value;
  }
  if (op == '!') {
    ConstValue result;
    result.type = ValueType::Int;
    if (value.type == ValueType::Float) {
      result.int_value = value.float_value == 0.0f ? 1 : 0;
    } else {
      result.int_value = value.int_value == 0 ? 1 : 0;
    }
    result.float_value = static_cast<float>(result.int_value);
    return result;
  }
  assert(false);
  return value;
}

ValueResult BinaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 二元表达式: 先生成左右值, 再输出对应二元指令
  auto *left = dynamic_cast<ExprAST *>(lhs.get());
  auto *right = dynamic_cast<ExprAST *>(rhs.get());
  assert(left != nullptr && right != nullptr);

  if (op == BinaryOp::And || op == BinaryOp::Or) {
    // 逻辑与/或: 使用分支实现短路求值
    ValueResult lhs_value = left->DumpKoopaValue(out);
    ValueResult lhs_bool = EmitNotZero(out, lhs_value);

    std::string result_alloc = NextAllocName("sc");
    out << "  " << result_alloc << " = alloc i32\n";
    RegisterValueType(result_alloc, ValueType::Int);

    std::string end_label = NextBlockLabel("sc_end");
    if (op == BinaryOp::And) {
      std::string rhs_label = NextBlockLabel("sc_and_rhs");
      std::string false_label = NextBlockLabel("sc_and_false");

      EmitBranch(out, lhs_bool.name, rhs_label, false_label);

      EmitLabel(out, rhs_label);
      ValueResult rhs_value = right->DumpKoopaValue(out);
      ValueResult rhs_bool = EmitNotZero(out, rhs_value);
      out << "  store " << rhs_bool.name << ", " << result_alloc << "\n";
      if (!IsBlockTerminated()) {
        EmitJump(out, end_label);
      }

      EmitLabel(out, false_label);
      out << "  store 0, " << result_alloc << "\n";
      if (!IsBlockTerminated()) {
        EmitJump(out, end_label);
      }
    } else {
      std::string true_label = NextBlockLabel("sc_or_true");
      std::string rhs_label = NextBlockLabel("sc_or_rhs");

      EmitBranch(out, lhs_bool.name, true_label, rhs_label);

      EmitLabel(out, true_label);
      out << "  store 1, " << result_alloc << "\n";
      if (!IsBlockTerminated()) {
        EmitJump(out, end_label);
      }

      EmitLabel(out, rhs_label);
      ValueResult rhs_value = right->DumpKoopaValue(out);
      ValueResult rhs_bool = EmitNotZero(out, rhs_value);
      out << "  store " << rhs_bool.name << ", " << result_alloc << "\n";
      if (!IsBlockTerminated()) {
        EmitJump(out, end_label);
      }
    }

    EmitLabel(out, end_label);
    return EmitLoad(out, result_alloc, ValueType::Int);
  }

  ValueResult lhs_value = left->DumpKoopaValue(out);
  ValueResult rhs_value = right->DumpKoopaValue(out);
  bool has_float = lhs_value.type == ValueType::Float || rhs_value.type == ValueType::Float;

  switch (op) {
    case BinaryOp::Add:
      return EmitBinary(out, "add", lhs_value.name, rhs_value.name,
                        has_float ? ValueType::Float : ValueType::Int);
    case BinaryOp::Sub:
      return EmitBinary(out, "sub", lhs_value.name, rhs_value.name,
                        has_float ? ValueType::Float : ValueType::Int);
    case BinaryOp::Mul:
      return EmitBinary(out, "mul", lhs_value.name, rhs_value.name,
                        has_float ? ValueType::Float : ValueType::Int);
    case BinaryOp::Div:
      return EmitBinary(out, "div", lhs_value.name, rhs_value.name,
                        has_float ? ValueType::Float : ValueType::Int);
    case BinaryOp::Mod:
      assert(!has_float);
      return EmitBinary(out, "mod", lhs_value.name, rhs_value.name, ValueType::Int);
    case BinaryOp::Lt:
      return EmitBinary(out, "lt", lhs_value.name, rhs_value.name, ValueType::Int);
    case BinaryOp::Gt:
      return EmitBinary(out, "gt", lhs_value.name, rhs_value.name, ValueType::Int);
    case BinaryOp::Le:
      return EmitBinary(out, "le", lhs_value.name, rhs_value.name, ValueType::Int);
    case BinaryOp::Ge:
      return EmitBinary(out, "ge", lhs_value.name, rhs_value.name, ValueType::Int);
    case BinaryOp::Eq:
      return EmitBinary(out, "eq", lhs_value.name, rhs_value.name, ValueType::Int);
    case BinaryOp::Ne:
      return EmitBinary(out, "ne", lhs_value.name, rhs_value.name, ValueType::Int);
    case BinaryOp::And:
    case BinaryOp::Or:
      break;
  }
  assert(false);
  return lhs_value;
}

ConstValue BinaryExpAST::EvalConst() const {
  auto *left = dynamic_cast<ExprAST *>(lhs.get());
  auto *right = dynamic_cast<ExprAST *>(rhs.get());
  assert(left != nullptr && right != nullptr);
  ConstValue lhs_value = left->EvalConst();
  ConstValue rhs_value = right->EvalConst();
  bool has_float = lhs_value.type == ValueType::Float || rhs_value.type == ValueType::Float;

  ConstValue result;
  switch (op) {
    case BinaryOp::Add:
      if (has_float) {
        result.type = ValueType::Float;
        result.float_value = ToFloat(lhs_value) + ToFloat(rhs_value);
        result.int_value = static_cast<int32_t>(result.float_value);
        return result;
      }
      result.type = ValueType::Int;
      result.int_value = lhs_value.int_value + rhs_value.int_value;
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Sub:
      if (has_float) {
        result.type = ValueType::Float;
        result.float_value = ToFloat(lhs_value) - ToFloat(rhs_value);
        result.int_value = static_cast<int32_t>(result.float_value);
        return result;
      }
      result.type = ValueType::Int;
      result.int_value = lhs_value.int_value - rhs_value.int_value;
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Mul:
      if (has_float) {
        result.type = ValueType::Float;
        result.float_value = ToFloat(lhs_value) * ToFloat(rhs_value);
        result.int_value = static_cast<int32_t>(result.float_value);
        return result;
      }
      result.type = ValueType::Int;
      result.int_value = lhs_value.int_value * rhs_value.int_value;
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Div:
      if (has_float) {
        result.type = ValueType::Float;
        result.float_value = ToFloat(lhs_value) / ToFloat(rhs_value);
        result.int_value = static_cast<int32_t>(result.float_value);
        return result;
      }
      result.type = ValueType::Int;
      result.int_value = lhs_value.int_value / rhs_value.int_value;
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Mod:
      assert(!has_float);
      result.type = ValueType::Int;
      result.int_value = lhs_value.int_value % rhs_value.int_value;
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Lt:
      result.type = ValueType::Int;
      result.int_value = has_float ? (ToFloat(lhs_value) < ToFloat(rhs_value) ? 1 : 0)
                                   : (lhs_value.int_value < rhs_value.int_value ? 1 : 0);
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Gt:
      result.type = ValueType::Int;
      result.int_value = has_float ? (ToFloat(lhs_value) > ToFloat(rhs_value) ? 1 : 0)
                                   : (lhs_value.int_value > rhs_value.int_value ? 1 : 0);
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Le:
      result.type = ValueType::Int;
      result.int_value = has_float ? (ToFloat(lhs_value) <= ToFloat(rhs_value) ? 1 : 0)
                                   : (lhs_value.int_value <= rhs_value.int_value ? 1 : 0);
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Ge:
      result.type = ValueType::Int;
      result.int_value = has_float ? (ToFloat(lhs_value) >= ToFloat(rhs_value) ? 1 : 0)
                                   : (lhs_value.int_value >= rhs_value.int_value ? 1 : 0);
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Eq:
      result.type = ValueType::Int;
      result.int_value = has_float ? (ToFloat(lhs_value) == ToFloat(rhs_value) ? 1 : 0)
                                   : (lhs_value.int_value == rhs_value.int_value ? 1 : 0);
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Ne:
      result.type = ValueType::Int;
      result.int_value = has_float ? (ToFloat(lhs_value) != ToFloat(rhs_value) ? 1 : 0)
                                   : (lhs_value.int_value != rhs_value.int_value ? 1 : 0);
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::And:
      result.type = ValueType::Int;
      result.int_value = (ToFloat(lhs_value) != 0.0f && ToFloat(rhs_value) != 0.0f) ? 1 : 0;
      result.float_value = static_cast<float>(result.int_value);
      return result;
    case BinaryOp::Or:
      result.type = ValueType::Int;
      result.int_value = (ToFloat(lhs_value) != 0.0f || ToFloat(rhs_value) != 0.0f) ? 1 : 0;
      result.float_value = static_cast<float>(result.int_value);
      return result;
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
  info.value_type = value_type;
  info.const_value = CastConstValue(expr->EvalConst(), value_type);
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
  info.value_type = value_type;
  info.alloc_name = NextAllocName(ident);
  out << "  " << info.alloc_name << " = alloc i32\n";
  RegisterValueType(info.alloc_name, value_type);
  InsertSymbol(ident, info);
  if (init) {
    auto *expr = dynamic_cast<ExprAST *>(init.get());
    assert(expr != nullptr);
    ValueResult init_value = expr->DumpKoopaValue(out);
    out << "  store " << init_value.name << ", " << info.alloc_name << "\n";
  }
}

void VarDeclAST::DumpKoopa(std::ostream &out) const {
  // 遍历变量定义
  for (const auto &def : defs) {
    def->DumpKoopa(out);
  }
}
