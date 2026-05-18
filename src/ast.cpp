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
// 函数参数类型信息: 存储每个参数是否为数组类型 (指针指向数组)
struct ParamTypeInfo {
  bool is_array_param = false;  // 是否为数组参数 (指针类型)
  std::vector<int> array_dims;  // 数组维度 (如 {10} 表示 *[i32, 10])
};
std::unordered_map<std::string, std::vector<ParamTypeInfo>> g_function_param_types;

struct SymbolInfo {
  enum class Kind { Const, Var, FuncParam, GlobalVar };
  Kind kind = Kind::Const;
  ValueType value_type = ValueType::Int;
  ConstValue const_value;
  std::string alloc_name;  // 局部变量/参数的 alloc 名; 全局变量则为全局符号名
  // 数组相关: 非空表示数组, 元素为各维长度 (如 {2, 3} 表示 int[2][3])
  std::vector<int> array_dims;
  // 是否为数组参数 (第一维省略, 如 int arr[] 或 int arr[][3])
  bool is_array_param = false;
};

std::vector<std::unordered_map<std::string, SymbolInfo>> g_scopes;

// 库函数声明信息
struct LibFuncInfo {
  std::string name;
  std::string param_types;  // e.g. "", "i32", "*i32", "i32, *i32"
  std::string ret_type;     // e.g. "i32" or "" for void
};

// SysY 标准库函数列表
// 注: getfloat/putfloat 在 Koopa IR 层面参数/返回值仍为 i32 (bit pattern),
// 仅在 RISC-V 后端按浮点调用约定使用 fa0/fa0-fa7
static const std::vector<LibFuncInfo> g_lib_funcs = {
  {"getint", "", "i32"},
  {"getch", "", "i32"},
  {"getfloat", "", "i32"},
  {"getarray", "*i32", "i32"},
  {"putint", "i32", ""},
  {"putch", "i32", ""},
  {"putfloat", "i32", ""},
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
void RegisterFunctionParamTypes(const std::string &name,
                                const std::vector<ParamTypeInfo> &params) {
  g_function_param_types[name] = params;
}

const std::vector<ParamTypeInfo> *LookupFunctionParamTypes(const std::string &name) {
  auto it = g_function_param_types.find(name);
  if (it != g_function_param_types.end()) {
    return &it->second;
  }
  return nullptr;
}

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
    } else if (func.ret_type == "float") {
      info.value_type = ValueType::Float;
    } else {
      info.value_type = ValueType::Int;
    }
    // getfloat 返回 float, 显式标记
    if (func.name == "getfloat") {
      info.value_type = ValueType::Float;
    }
    info.alloc_name = "@" + func.name;
    global_scope[func.name] = info;
    RegisterFunctionReturnType(func.name, info.value_type);

    // 注册库函数参数类型 (用于函数调用时的类型匹配)
    std::vector<ParamTypeInfo> param_types;
    if (func.name == "getarray") {
      param_types.push_back({true, {}});  // *i32 (int[])
    } else if (func.name == "putarray") {
      param_types.push_back({false, {}});  // i32
      param_types.push_back({true, {}});   // *i32 (int[])
    }
    if (!param_types.empty()) {
      RegisterFunctionParamTypes(func.name, param_types);
    }
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

// 求值维度表达式列表, 返回维度数组
std::vector<int> EvalDimExprs(const std::vector<std::unique_ptr<BaseAST>> &dim_exprs) {
  std::vector<int> dims;
  for (const auto &expr_ast : dim_exprs) {
    auto *expr = dynamic_cast<ExprAST *>(expr_ast.get());
    assert(expr != nullptr);
    ConstValue val = expr->EvalConst();
    dims.push_back(val.int_value);
  }
  return dims;
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

// 生成 Koopa IR 数组类型字符串
// 如 dims={2,3} 生成 "[[i32, 3], 2]", dims={4} 生成 "[i32, 4]"
std::string MakeArrayType(const std::vector<int> &dims) {
  if (dims.empty()) return "i32";
  std::string inner = "i32";
  // 从最内层维度向外构建类型
  for (int i = static_cast<int>(dims.size()) - 1; i >= 0; --i) {
    inner = "[" + inner + ", " + std::to_string(dims[i]) + "]";
  }
  return inner;
}

// 计算数组类型的元素大小 (字节数)
// 如 dims={2,3}, elem_size=4 -> 12; dims={3}, elem_size=4 -> 4
int CalcArrayElemSize(const std::vector<int> &dims, int dim_start) {
  int size = 4;  // i32 基础大小
  for (int i = static_cast<int>(dims.size()) - 1; i > dim_start; --i) {
    size *= dims[i];
  }
  return size;
}

// 计算数组总元素个数
int CalcArrayTotalSize(const std::vector<int> &dims) {
  int total = 1;
  for (int d : dims) total *= d;
  return total;
}

// 将初始化列表展平为一维数组 (递归处理嵌套列表)
// init: 初始化表达式 (ExprAST 或 InitValListAST)
// dims: 数组维度列表
// dim_start: 当前处理的维度起始索引
// flat: 输出的展平数组
// pos: 当前写入位置 (按引用传递)
void FlattenArrayInit(BaseAST *init, const std::vector<int> &dims, int dim_start,
                      std::vector<int32_t> &flat, int &pos) {
  auto *list = dynamic_cast<InitValListAST *>(init);

  // 达到标量层级或非列表初始化
  if (dim_start >= static_cast<int>(dims.size()) || !list) {
    auto *expr = dynamic_cast<ExprAST *>(init);
    assert(expr != nullptr);
    ConstValue val = expr->EvalConst();
    if (val.type == ValueType::Float) {
      flat[pos++] = FloatToBits(val.float_value);
    } else {
      flat[pos++] = val.int_value;
    }
    return;
  }

  // 计算当前子数组的总大小
  int total_size = 1;
  for (int k = dim_start; k < static_cast<int>(dims.size()); ++k) {
    total_size *= dims[k];
  }

  int filled = 0;
  for (auto &item : list->items) {
    if (filled >= total_size) break;

    auto *sub = dynamic_cast<InitValListAST *>(item.get());
    if (!sub) {
      // 标量元素
      auto *expr = dynamic_cast<ExprAST *>(item.get());
      assert(expr != nullptr);
      ConstValue val = expr->EvalConst();
      if (val.type == ValueType::Float) {
        flat[pos + filled] = FloatToBits(val.float_value);
      } else {
        flat[pos + filled] = val.int_value;
      }
      filled++;
    } else {
      // 子列表: 确定其对应的维度层级
      // 查找 pos+filled 对齐到的最大维度边界
      int sub_stride = 1;
      int actual_dim = static_cast<int>(dims.size()) - 1;
      for (int k = static_cast<int>(dims.size()) - 1; k >= dim_start; --k) {
        if ((pos + filled) % sub_stride == 0) {
          actual_dim = k;
        }
        sub_stride *= dims[k];
      }
      // 递归展平子列表
      int sub_pos = pos + filled;
      FlattenArrayInit(item.get(), dims, actual_dim + 1, flat, sub_pos);
      // 计算子数组大小并推进 filled
      int sub_size = 1;
      for (int k = actual_dim + 1; k < static_cast<int>(dims.size()); ++k) {
        sub_size *= dims[k];
      }
      filled += sub_size;
    }
  }
}

// 为数组元素生成多级 getelemptr 指令链, 返回最终的元素指针
// dims: 数组维度列表, flat_idx: 展平后的索引
std::string EmitArrayElemPtr(std::ostream &out, const std::string &arr_name,
                             const std::vector<int> &dims, int flat_idx,
                             ValueType elem_type) {
  std::string cur_ptr = arr_name;
  int remaining = flat_idx;
  // 逐维计算索引并生成 getelemptr
  for (size_t d = 0; d < dims.size(); ++d) {
    int stride = 1;
    for (size_t k = d + 1; k < dims.size(); ++k) {
      stride *= dims[k];
    }
    int idx = remaining / stride;
    remaining %= stride;
    std::string next_ptr = NextTemp(elem_type);
    out << "  " << next_ptr << " = getelemptr " << cur_ptr << ", " << idx << "\n";
    RegisterValueType(next_ptr, elem_type);
    cur_ptr = next_ptr;
  }
  return cur_ptr;
}
// dims: 数组维度, flat: 展平的初始值数组
std::string MakeAggregateInit(const std::vector<int> &dims, int dim_start,
                              const std::vector<int32_t> &flat, int &pos) {
  if (dim_start >= static_cast<int>(dims.size())) {
    // 标量元素
    return std::to_string(flat[pos++]);
  }
  std::string result = "{";
  int count = dims[dim_start];
  for (int i = 0; i < count; ++i) {
    if (i > 0) result += ", ";
    result += MakeAggregateInit(dims, dim_start + 1, flat, pos);
  }
  result += "}";
  return result;
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
  // 赋值语句: 计算右值并写回变量 (支持标量和数组元素赋值)
  auto *lhs = dynamic_cast<LValAST *>(lval.get());
  auto *rhs = dynamic_cast<ExprAST *>(value.get());
  assert(lhs != nullptr && rhs != nullptr);
  SymbolInfo *info = LookupSymbol(lhs->ident);
  assert(info != nullptr && (info->kind == SymbolInfo::Kind::Var ||
         info->kind == SymbolInfo::Kind::FuncParam ||
         info->kind == SymbolInfo::Kind::GlobalVar));
  ValueResult rhs_value = rhs->DumpKoopaValue(out);

  // 计算目标地址
  if (lhs->indices.empty()) {
    // 标量赋值
    out << "  store " << rhs_value.name << ", " << info->alloc_name << "\n";
  } else {
    // 数组元素赋值: 计算元素地址
    std::string cur_ptr;
    if (info->is_array_param) {
      // 数组参数: getptr + getelemptr
      std::string ptr = EmitLoad(out, info->alloc_name, info->value_type).name;
      auto *first_idx = dynamic_cast<ExprAST *>(lhs->indices[0].get());
      ValueResult first_val = first_idx->DumpKoopaValue(out);
      cur_ptr = NextTemp(info->value_type);
      out << "  " << cur_ptr << " = getptr " << ptr << ", " << first_val.name << "\n";
      RegisterValueType(cur_ptr, info->value_type);
      for (size_t i = 1; i < lhs->indices.size(); ++i) {
        auto *idx_expr = dynamic_cast<ExprAST *>(lhs->indices[i].get());
        ValueResult idx_val = idx_expr->DumpKoopaValue(out);
        std::string next_ptr = NextTemp(info->value_type);
        out << "  " << next_ptr << " = getelemptr " << cur_ptr << ", " << idx_val.name << "\n";
        RegisterValueType(next_ptr, info->value_type);
        cur_ptr = next_ptr;
      }
    } else {
      // 普通数组: getelemptr
      cur_ptr = info->alloc_name;
      for (size_t i = 0; i < lhs->indices.size(); ++i) {
        auto *idx_expr = dynamic_cast<ExprAST *>(lhs->indices[i].get());
        ValueResult idx_val = idx_expr->DumpKoopaValue(out);
        std::string next_ptr = NextTemp(info->value_type);
        out << "  " << next_ptr << " = getelemptr " << cur_ptr << ", " << idx_val.name << "\n";
        RegisterValueType(next_ptr, info->value_type);
        cur_ptr = next_ptr;
      }
    }
    out << "  store " << rhs_value.name << ", " << cur_ptr << "\n";
  }
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
  // 使用唯一参数名避免与全局变量冲突 (如全局 int n 与参数 int n)
  // 先生成所有参数名, 确保 header 和 body 一致
  std::vector<std::string> param_ir_names;
  out << "fun @" << ident << "(";
  for (size_t i = 0; i < params.size(); ++i) {
    if (i > 0) out << ", ";
    auto *param = dynamic_cast<FuncFParamAST *>(params[i].get());
    assert(param != nullptr);
    // 使用 @p_i 格式确保唯一性, 不与 alloc 名称冲突
    std::string param_name = "@p" + std::to_string(i);
    param_ir_names.push_back(param_name);
    out << param_name << ": ";
    if (param->is_array) {
      std::vector<int> dims = EvalDimExprs(param->dim_exprs);
      if (dims.empty()) {
        out << "*i32";
      } else {
        out << "*" << MakeArrayType(dims);
      }
    } else {
      out << "i32";
    }
  }
  out << ")";
  if (ret_type != ValueType::Void) {
    out << ": i32";
  }
  out << " {\n";

  // 注册函数参数类型信息 (用于函数调用时的类型匹配)
  {
    std::vector<ParamTypeInfo> param_types;
    for (const auto &param : params) {
      auto *p = dynamic_cast<FuncFParamAST *>(param.get());
      ParamTypeInfo pti;
      pti.is_array_param = p->is_array;
      if (p->is_array) {
        pti.array_dims = EvalDimExprs(p->dim_exprs);
      }
      param_types.push_back(pti);
    }
    RegisterFunctionParamTypes(ident, param_types);
  }

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
  for (size_t pi = 0; pi < params.size(); ++pi) {
    auto *p = dynamic_cast<FuncFParamAST *>(params[pi].get());
    assert(p != nullptr);
    SymbolInfo info;
    info.kind = SymbolInfo::Kind::FuncParam;
    info.value_type = p->value_type;
    info.is_array_param = p->is_array;
    info.array_dims = EvalDimExprs(p->dim_exprs);

    // 使用与 header 一致的参数名
    const std::string &param_ir_name = param_ir_names[pi];

    if (p->is_array) {
      // 数组参数: 分配指针类型的局部变量
      std::string ptr_type;
      if (info.array_dims.empty()) {
        ptr_type = "*i32";
      } else {
        ptr_type = "*" + MakeArrayType(info.array_dims);
      }
      info.alloc_name = NextAllocName(p->ident);
      out << "  " << info.alloc_name << " = alloc " << ptr_type << "\n";
      RegisterValueType(info.alloc_name, p->value_type);
      out << "  store " << param_ir_name << ", " << info.alloc_name << "\n";
    } else {
      // 标量参数
      info.alloc_name = NextAllocName(p->ident);
      out << "  " << info.alloc_name << " = alloc i32\n";
      RegisterValueType(info.alloc_name, p->value_type);
      out << "  store " << param_ir_name << ", " << info.alloc_name << "\n";
    }
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
  // 读取变量或常量: 支持标量和数组元素访问
  SymbolInfo *info = LookupSymbol(ident);
  assert(info != nullptr);

  // 常量标量: 直接返回编译期求值的立即数
  if (info->kind == SymbolInfo::Kind::Const && info->array_dims.empty()) {
    if (info->value_type == ValueType::Int) {
      return {std::to_string(info->const_value.int_value), ValueType::Int};
    }
    int32_t bits = FloatToBits(info->const_value.float_value);
    return EmitBinary(out, "add", "0", std::to_string(bits), ValueType::Float);
  }

  // 数组参数 (指针类型): 使用 getptr + getelemptr 访问
  if (info->is_array_param) {
    // 加载参数指针
    std::string ptr = EmitLoad(out, info->alloc_name, info->value_type).name;

    if (indices.empty()) {
      // 无下标: 返回数组首地址 (指针)
      // 对于数组参数, 无下标时也需要标记剩余维度 (用于函数调用时的类型匹配)
      bool need_decay = !info->array_dims.empty();
      return {ptr, info->value_type, need_decay, info->array_dims};
    }

    // 有下标: 先用 getptr 处理第一维, 再用 getelemptr 处理后续维度
    auto *first_idx = dynamic_cast<ExprAST *>(indices[0].get());
    assert(first_idx != nullptr);
    ValueResult first_val = first_idx->DumpKoopaValue(out);
    std::string cur_ptr = NextTemp(info->value_type);
    out << "  " << cur_ptr << " = getptr " << ptr << ", " << first_val.name << "\n";
    RegisterValueType(cur_ptr, info->value_type);

    // 后续维度使用 getelemptr
    for (size_t i = 1; i < indices.size(); ++i) {
      auto *idx_expr = dynamic_cast<ExprAST *>(indices[i].get());
      assert(idx_expr != nullptr);
      ValueResult idx_val = idx_expr->DumpKoopaValue(out);
      std::string next_ptr = NextTemp(info->value_type);
      out << "  " << next_ptr << " = getelemptr " << cur_ptr << ", " << idx_val.name << "\n";
      RegisterValueType(next_ptr, info->value_type);
      cur_ptr = next_ptr;
    }

    // 如果下标数等于数组维数+1 (到达元素), 加载元素值
    if (indices.size() == info->array_dims.size() + 1) {
      return EmitLoad(out, cur_ptr, info->value_type);
    }
    // 否则返回子数组指针 (标记为数组指针)
    // 对于数组参数, getptr 保持类型不变, 剩余维度始终是 array_dims
    return {cur_ptr, info->value_type, true, info->array_dims};
  }

  // 普通数组变量: 使用 getelemptr 访问
  if (!info->array_dims.empty()) {
    if (indices.empty()) {
      // 无下标: 返回数组首地址 (用于函数传参等场景)
      std::string elem_ptr = NextTemp(info->value_type);
      out << "  " << elem_ptr << " = getelemptr " << info->alloc_name << ", 0\n";
      RegisterValueType(elem_ptr, info->value_type);
      // 无下标时结果类型已经是正确的指针类型, 但维度比原数组少一层
      std::vector<int> rem(info->array_dims.begin() + 1, info->array_dims.end());
      bool need_decay = !rem.empty();  // 如果还有剩余维度, 需要衰减
      return {elem_ptr, info->value_type, need_decay, rem};
    }

    // 有下标: 链式 getelemptr
    std::string cur_ptr = info->alloc_name;
    for (size_t i = 0; i < indices.size(); ++i) {
      auto *idx_expr = dynamic_cast<ExprAST *>(indices[i].get());
      assert(idx_expr != nullptr);
      ValueResult idx_val = idx_expr->DumpKoopaValue(out);
      std::string next_ptr = NextTemp(info->value_type);
      out << "  " << next_ptr << " = getelemptr " << cur_ptr << ", " << idx_val.name << "\n";
      RegisterValueType(next_ptr, info->value_type);
      cur_ptr = next_ptr;
    }

    // 如果下标数等于数组维数, 加载元素值 (normal arrays)
    if (indices.size() == info->array_dims.size()) {
      return EmitLoad(out, cur_ptr, info->value_type);
    }
    // 否则返回子数组指针 (标记为数组指针, 记录剩余维度)
    std::vector<int> rem(info->array_dims.begin() + indices.size(),
                         info->array_dims.end());
    return {cur_ptr, info->value_type, true, rem};
  }

  // 标量变量
  if (info->kind == SymbolInfo::Kind::GlobalVar) {
    return EmitLoad(out, info->alloc_name, info->value_type);
  }
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
    const auto *param_types = LookupFunctionParamTypes(call_ident);
    for (size_t i = 0; i < call_args.size(); ++i) {
      auto *expr = dynamic_cast<ExprAST *>(call_args[i].get());
      assert(expr != nullptr);
      ValueResult arg_val = expr->DumpKoopaValue(out);
      // 如果参数是数组指针, 检查是否需要衰减
      if (arg_val.is_array_ptr) {
        // 计算需要衰减的层数
        int arg_depth = arg_val.remaining_dims.size();
        int expected_depth = 0;
        if (param_types && i < param_types->size()) {
          const auto &expected = (*param_types)[i];
          if (expected.is_array_param) {
            expected_depth = expected.array_dims.size();
          }
        }
        // 如果参数维度多于期望维度, 需要逐层衰减
        while (arg_depth > expected_depth) {
          std::string decayed = NextTemp(arg_val.type);
          out << "  " << decayed << " = getelemptr " << arg_val.name << ", 0\n";
          RegisterValueType(decayed, arg_val.type);
          arg_val.name = decayed;
          arg_val.remaining_dims.erase(arg_val.remaining_dims.begin());
          arg_depth--;
        }
        if (arg_depth == 0) {
          arg_val.is_array_ptr = false;
        }
      }
      args.push_back(arg_val);
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
  // 常量定义: 标量在编译期求值, 数组需要生成 IR
  SymbolInfo info;
  info.kind = SymbolInfo::Kind::Const;
  info.value_type = value_type;
  info.array_dims = EvalDimExprs(dim_exprs);

  if (info.array_dims.empty()) {
    // 标量常量: 编译期求值
    auto *expr = dynamic_cast<ExprAST *>(init.get());
    assert(expr != nullptr);
    info.const_value = CastConstValue(expr->EvalConst(), value_type);
    InsertSymbol(ident, info);
  } else {
    // 常量数组: 需要生成 IR (const 仅表示不可修改)
    // 在 IR 层面与普通数组相同, 使用 alloc + store
    info.kind = g_in_global ? SymbolInfo::Kind::GlobalVar : SymbolInfo::Kind::Var;
    std::string koopa_type = MakeArrayType(info.array_dims);

    if (g_in_global) {
      info.alloc_name = "@" + ident;
      // 全局常量数组: 展平并生成聚合常量
      std::vector<int32_t> flat(CalcArrayTotalSize(info.array_dims), 0);
      int pos = 0;
      FlattenArrayInit(init.get(), info.array_dims, 0, flat, pos);
      int agg_pos = 0;
      std::string agg = MakeAggregateInit(info.array_dims, 0, flat, agg_pos);
      out << "global " << info.alloc_name << " = alloc " << koopa_type << ", " << agg << "\n";
    } else {
      // 局部常量数组
      info.alloc_name = NextAllocName(ident);
      out << "  " << info.alloc_name << " = alloc " << koopa_type << "\n";
      RegisterValueType(info.alloc_name, value_type);

      // 展平并逐元素 store (使用多级 getelemptr)
      int total = CalcArrayTotalSize(info.array_dims);
      std::vector<int32_t> flat(total, 0);
      int pos = 0;
      FlattenArrayInit(init.get(), info.array_dims, 0, flat, pos);

      for (int i = 0; i < total; ++i) {
        std::string elem_ptr = EmitArrayElemPtr(out, info.alloc_name, info.array_dims, i, value_type);
        out << "  store " << flat[i] << ", " << elem_ptr << "\n";
      }
    }
    RegisterValueType(info.alloc_name, value_type);
    InsertSymbol(ident, info);
  }
}

void ConstDeclAST::DumpKoopa(std::ostream &out) const {
  // 遍历常量定义
  for (const auto &def : defs) {
    def->DumpKoopa(out);
  }
}

void VarDefAST::DumpKoopa(std::ostream &out) const {
  // 全局/局部变量定义: 支持标量和数组
  SymbolInfo info;
  info.kind = g_in_global ? SymbolInfo::Kind::GlobalVar : SymbolInfo::Kind::Var;
  info.value_type = value_type;
  info.array_dims = EvalDimExprs(dim_exprs);

  std::string koopa_type = info.array_dims.empty() ? "i32" : MakeArrayType(info.array_dims);

  if (g_in_global) {
    // 全局变量
    info.alloc_name = "@" + ident;
    if (!info.array_dims.empty()) {
      // 全局数组
      if (init) {
        // 有初始值: 展平并生成聚合常量
        std::vector<int32_t> flat(CalcArrayTotalSize(info.array_dims), 0);
        int pos = 0;
        FlattenArrayInit(init.get(), info.array_dims, 0, flat, pos);
        int agg_pos = 0;
        std::string agg = MakeAggregateInit(info.array_dims, 0, flat, agg_pos);
        out << "global " << info.alloc_name << " = alloc " << koopa_type << ", " << agg << "\n";
      } else {
        // 未初始化: 零初始化
        out << "global " << info.alloc_name << " = alloc " << koopa_type << ", zeroinit\n";
      }
    } else {
      // 全局标量
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
        out << "global " << info.alloc_name << " = alloc i32, zeroinit\n";
      }
    }
    RegisterValueType(info.alloc_name, value_type);
  } else {
    // 局部变量
    info.alloc_name = NextAllocName(ident);
    out << "  " << info.alloc_name << " = alloc " << koopa_type << "\n";
    RegisterValueType(info.alloc_name, value_type);

    if (!info.array_dims.empty()) {
      // 局部数组
      if (init) {
        // 展平初始化列表
        int total = CalcArrayTotalSize(info.array_dims);
        std::vector<int32_t> flat(total, 0);
        int pos = 0;
        FlattenArrayInit(init.get(), info.array_dims, 0, flat, pos);

        // 逐元素 store (使用多级 getelemptr)
        for (int i = 0; i < total; ++i) {
          std::string elem_ptr = EmitArrayElemPtr(out, info.alloc_name, info.array_dims, i, value_type);
          out << "  store " << flat[i] << ", " << elem_ptr << "\n";
        }
      }
      // 未初始化的局部数组不需要显式零初始化 (栈上默认为 0)
    } else {
      // 局部标量
      if (init) {
        auto *expr = dynamic_cast<ExprAST *>(init.get());
        assert(expr != nullptr);
        ValueResult init_value = expr->DumpKoopaValue(out);
        out << "  store " << init_value.name << ", " << info.alloc_name << "\n";
      }
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

void InitValListAST::DumpKoopa(std::ostream &out) const {
  // 初始化列表不直接输出 IR, 由父节点 (VarDefAST/ConstDefAST) 处理
  (void)out;
}
