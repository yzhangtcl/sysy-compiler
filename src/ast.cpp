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
// 当前正在生成的函数的返回类型, 用于 return 语句
ValueType g_current_func_return_type = ValueType::Int;
std::unordered_map<std::string, ValueType> g_value_types;
std::unordered_map<std::string, ValueType> g_function_return_types;

struct SymbolInfo {
  enum class Kind { Const, Var, FuncParam, GlobalVar };
  Kind kind = Kind::Const;
  ValueType value_type = ValueType::Int;
  ConstValue const_value;
  std::string alloc_name;  // 局部变量/参数的 alloc 名; 全局变量则为全局符号名
};

std::vector<std::unordered_map<std::string, SymbolInfo>> g_scopes;

// 库函数声明信息
struct LibFuncInfo {
  std::string name;
  std::string param_types;  // e.g. "", "i32", "*i32", "i32, *i32"
  std::string ret_type;     // e.g. "i32" or "" for void
};

// SysY 标准库函数列表
static const std::vector<LibFuncInfo> g_lib_funcs = {
  {"getint", "", "i32"},
  {"getch", "", "i32"},
  {"getarray", "*i32", "i32"},
  {"putint", "i32", ""},
  {"putch", "i32", ""},
  {"putarray", "i32, *i32", ""},
  {"starttime", "", ""},
  {"stoptime", "", ""},
};

struct LoopContext {
  std::string entry_label;
  std::string end_label;
};

std::vector<LoopContext> g_loop_stack;

// 输出所有 SysY 库函数的 decl 声明
void EmitLibFuncDecls(std::ostream &out) {
  for (const auto &func : g_lib_funcs) {
    out << "decl @" << func.name << "(" << func.param_types << ")";
    if (!func.ret_type.empty()) {
      out << ": " << func.ret_type;
    }
    out << "\n";
  }
}

// 将 SysY 库函数信息注册到全局符号表
void RegisterLibFuncs() {
  assert(!g_scopes.empty());
  auto &global_scope = g_scopes.front();
  for (const auto &func : g_lib_funcs) {
    SymbolInfo info;
    info.kind = SymbolInfo::Kind::Var;  // 用 Var 标记以免被当成常量
    if (func.ret_type == "i32") {
      info.value_type = ValueType::Int;
    } else if (func.ret_type.empty()) {
      info.value_type = ValueType::Void;
    } else {
      info.value_type = ValueType::Int;
    }
    info.alloc_name = "@" + func.name;
    global_scope[func.name] = info;
    RegisterFunctionReturnType(func.name, info.value_type);
  }
}

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

const LoopContext &CurrentLoop() {
  // break/continue 需要确保处于循环上下文中
  assert(!g_loop_stack.empty());
  return g_loop_stack.back();
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

// 标记当前是否在全局作用域内
static bool g_in_global = false;

void CompUnitAST::DumpKoopa(std::ostream &out) const {
  // 编译单元: 按顺序输出全局声明与函数定义
  // 1. 建立全局作用域
  g_scopes.clear();
  EnterScope();  // 全局作用域
  RegisterLibFuncs();

  // 2. 预注册所有函数名, 允许函数间相互调用 (无论定义顺序)
  for (const auto &item : items) {
    auto *func_def = dynamic_cast<FuncDefAST *>(item.get());
    if (func_def) {
      auto *type_ast = dynamic_cast<FuncTypeAST *>(func_def->func_type.get());
      ValueType ret_type = type_ast ? type_ast->value_type : ValueType::Int;
      SymbolInfo info;
      info.kind = SymbolInfo::Kind::Var;  // 函数符号
      info.value_type = ret_type;
      info.alloc_name = "@" + func_def->ident;
      RegisterFunctionReturnType(func_def->ident, ret_type);
      InsertSymbol(func_def->ident, info);
    }
  }

  // 3. 输出库函数 IR 声明
  EmitLibFuncDecls(out);

  // 4. 先处理所有全局声明 (Decl: VarDecl / ConstDecl)
  g_in_global = true;
  for (const auto &item : items) {
    auto *var_decl = dynamic_cast<VarDeclAST *>(item.get());
    if (var_decl) {
      var_decl->DumpKoopa(out);
      continue;
    }
    auto *const_decl = dynamic_cast<ConstDeclAST *>(item.get());
    if (const_decl) {
      const_decl->DumpKoopa(out);
      continue;
    }
  }
  g_in_global = false;

  // 5. 再输出所有函数定义
  for (const auto &item : items) {
    auto *func_def = dynamic_cast<FuncDefAST *>(item.get());
    if (func_def) {
      func_def->DumpKoopa(out);
    }
  }

  ExitScope();  // 退出全局作用域
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
  // 输出 return 指令: 有返回值时输出 ret value, void 时仅输出 ret
  if (ret_exp) {
    auto *expr = dynamic_cast<ExprAST *>(ret_exp.get());
    assert(expr != nullptr);
    ValueResult value = expr->DumpKoopaValue(out);
    out << "  ret " << value.name << "\n";
  } else {
    // void 函数返回
    out << "  ret\n";
  }
  g_block_terminated = true;
}

void AssignStmtAST::DumpKoopa(std::ostream &out) const {
  // 赋值语句: 计算右值并写回变量 (支持全局/局部变量)
  auto *lhs = dynamic_cast<LValAST *>(lval.get());
  auto *rhs = dynamic_cast<ExprAST *>(value.get());
  assert(lhs != nullptr && rhs != nullptr);
  SymbolInfo *info = LookupSymbol(lhs->ident);
  assert(info != nullptr && (info->kind == SymbolInfo::Kind::Var ||
         info->kind == SymbolInfo::Kind::FuncParam ||
         info->kind == SymbolInfo::Kind::GlobalVar));
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

void WhileStmtAST::DumpKoopa(std::ostream &out) const {
  // while 语句: 生成条件块、循环体与结束块
  auto *cond_expr = dynamic_cast<ExprAST *>(cond.get());
  assert(cond_expr != nullptr);

  std::string entry_label = NextBlockLabel("while_entry");
  std::string body_label = NextBlockLabel("while_body");
  std::string end_label = NextBlockLabel("while_end");

  EmitJump(out, entry_label);

  EmitLabel(out, entry_label);
  ValueResult cond_value = cond_expr->DumpKoopaValue(out);
  ValueResult cond_bool = EmitNotZero(out, cond_value);
  EmitBranch(out, cond_bool.name, body_label, end_label);

  EmitLabel(out, body_label);
  // 记录当前循环的入口与退出位置, 供 break/continue 使用
  g_loop_stack.push_back({entry_label, end_label});
  body->DumpKoopa(out);
  g_loop_stack.pop_back();
  if (!IsBlockTerminated()) {
    EmitJump(out, entry_label);
  }

  EmitLabel(out, end_label);
}

void BreakStmtAST::DumpKoopa(std::ostream &out) const {
  // break: 跳转到最近一层循环的结束块
  (void)out;
  const auto &loop = CurrentLoop();
  EmitJump(out, loop.end_label);
}

void ContinueStmtAST::DumpKoopa(std::ostream &out) const {
  // continue: 跳转到最近一层循环的条件块
  (void)out;
  const auto &loop = CurrentLoop();
  EmitJump(out, loop.entry_label);
}

void FuncDefAST::DumpKoopa(std::ostream &out) const {
  // 输出函数定义: fun @name(@param: type, ...): ret_type { ... }
  auto *type_ast = dynamic_cast<FuncTypeAST *>(func_type.get());
  ValueType ret_type = type_ast ? type_ast->value_type : ValueType::Int;
  g_current_func_return_type = ret_type;
  RegisterFunctionReturnType(ident, ret_type);

  // 输出函数头
  out << "fun @" << ident << "(";
  for (size_t i = 0; i < params.size(); ++i) {
    if (i > 0) out << ", ";
    auto *param = dynamic_cast<FuncFParamAST *>(params[i].get());
    assert(param != nullptr);
    out << "@" << param->ident << ": i32";
  }
  out << ")";
  if (ret_type != ValueType::Void) {
    out << ": i32";
  }
  out << " {\n";

  // 每个函数从 0 开始编号临时变量
  g_temp_id = 0;
  g_alloc_id = 0;
  g_block_id = 0;
  g_block_terminated = false;
  g_loop_stack.clear();
  ResetValueTypeTable();

  // 建立函数作用域: 全局作用域已在 CompUnit 中设置, 此处进入函数局部作用域
  EnterScope();

  EmitLabel(out, "%entry");

  // 为每个参数分配局部内存, 并把参数值 store 进去
  for (const auto &param : params) {
    auto *p = dynamic_cast<FuncFParamAST *>(param.get());
    assert(p != nullptr);
    SymbolInfo info;
    info.kind = SymbolInfo::Kind::FuncParam;
    info.value_type = p->value_type;
    info.alloc_name = NextAllocName(p->ident);
    out << "  " << info.alloc_name << " = alloc i32\n";
    RegisterValueType(info.alloc_name, p->value_type);
    // 将参数值存入局部 alloc
    out << "  store @" << p->ident << ", " << info.alloc_name << "\n";
    InsertSymbol(p->ident, info);
  }

  // 输出函数体
  block->DumpKoopa(out);

  // 若函数体末尾未被 return/br/jump 终结, 补一个 ret
  if (!IsBlockTerminated()) {
    if (ret_type == ValueType::Void) {
      out << "  ret\n";
    } else {
      // 非 void 函数隐式返回 0 (符合 SysY 规范)
      out << "  ret 0\n";
    }
  }

  ExitScope();  // 退出函数局部作用域

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
  // 读取变量或常量: 全局变量直接用 @name, 局部变量通过 alloc+load
  SymbolInfo *info = LookupSymbol(ident);
  assert(info != nullptr);
  if (info->kind == SymbolInfo::Kind::Const) {
    // 常量: 直接返回编译期求值的立即数
    if (info->value_type == ValueType::Int) {
      return {std::to_string(info->const_value.int_value), ValueType::Int};
    }
    int32_t bits = FloatToBits(info->const_value.float_value);
    return EmitBinary(out, "add", "0", std::to_string(bits), ValueType::Float);
  }
  // 全局变量: 直接用 @name 做 load
  if (info->kind == SymbolInfo::Kind::GlobalVar) {
    return EmitLoad(out, info->alloc_name, info->value_type);
  }
  // 局部变量或参数: 通过 alloc 名 load
  return EmitLoad(out, info->alloc_name, info->value_type);
}

ConstValue LValAST::EvalConst() const {
  SymbolInfo *info = LookupSymbol(ident);
  assert(info != nullptr && info->kind == SymbolInfo::Kind::Const);
  return info->const_value;
}

ValueResult UnaryExpAST::DumpKoopaValue(std::ostream &out) const {
  // 函数调用: call @name(args)
  if (!call_ident.empty()) {
    SymbolInfo *func_info = LookupSymbol(call_ident);
    assert(func_info != nullptr);
    ValueType ret_type = func_info->value_type;

    // 先计算所有实参的值
    std::vector<ValueResult> args;
    for (const auto &arg : call_args) {
      auto *expr = dynamic_cast<ExprAST *>(arg.get());
      assert(expr != nullptr);
      args.push_back(expr->DumpKoopaValue(out));
    }

    // 输出 call 指令
    out << "  ";
    if (ret_type != ValueType::Void) {
      std::string result = NextTemp(ret_type);
      out << result << " = ";
      out << "call @" << call_ident << "(";
      for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) out << ", ";
        out << args[i].name;
      }
      out << ")\n";
      RegisterValueType(result, ret_type);
      return {result, ret_type};
    } else {
      // void 函数调用: 无返回值, 直接 call
      out << "call @" << call_ident << "(";
      for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) out << ", ";
        out << args[i].name;
      }
      out << ")\n";
      // void 调用返回一个 dummy 值 (不会被使用)
      return {"0", ValueType::Void};
    }
  }

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
  // 函数调用不可在编译期求值
  assert(call_ident.empty());
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
  // 全局/局部变量定义: 根据作用域生成不同的分配指令
  SymbolInfo info;
  info.kind = g_in_global ? SymbolInfo::Kind::GlobalVar : SymbolInfo::Kind::Var;
  info.value_type = value_type;

  if (g_in_global) {
    // 全局变量: 使用 global alloc, 必须有初始值
    info.alloc_name = "@" + ident;
    // 计算初始值
    if (init) {
      auto *expr = dynamic_cast<ExprAST *>(init.get());
      assert(expr != nullptr);
      ConstValue init_value = CastConstValue(expr->EvalConst(), value_type);
      if (value_type == ValueType::Float) {
        int32_t bits = FloatToBits(init_value.float_value);
        out << "global " << info.alloc_name << " = alloc i32, " << bits << "\n";
      } else {
        out << "global " << info.alloc_name << " = alloc i32, " << init_value.int_value << "\n";
      }
    } else {
      // 未初始化全局变量: 使用 zeroinit
      out << "global " << info.alloc_name << " = alloc i32, zeroinit\n";
    }
    RegisterValueType(info.alloc_name, value_type);
  } else {
    // 局部变量: 使用 alloc
    info.alloc_name = NextAllocName(ident);
    out << "  " << info.alloc_name << " = alloc i32\n";
    RegisterValueType(info.alloc_name, value_type);
    if (init) {
      auto *expr = dynamic_cast<ExprAST *>(init.get());
      assert(expr != nullptr);
      ValueResult init_value = expr->DumpKoopaValue(out);
      out << "  store " << init_value.name << ", " << info.alloc_name << "\n";
    }
  }
  InsertSymbol(ident, info);
}

void VarDeclAST::DumpKoopa(std::ostream &out) const {
  // 遍历变量定义
  for (const auto &def : defs) {
    def->DumpKoopa(out);
  }
}

void FuncFParamAST::DumpKoopa(std::ostream &out) const {
  // 形式参数由 FuncDefAST 统一处理, 本节点不单独输出 IR
  (void)out;
}
