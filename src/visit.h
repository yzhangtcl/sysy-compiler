#pragma once

#include <iosfwd>
#include <string>
#include <unordered_map>

#include "ast.h"
#include "koopa.h"

class AsmGenerator {
 public:
  // 入口: 从 raw program 生成 RISC-V 汇编
  void Generate(const koopa_raw_program_t &program, std::ostream &out);

 private:
  // 访问 raw slice, 按元素类型分发
  void VisitSlice(const koopa_raw_slice_t &slice, std::ostream &out);
  // 访问函数并输出函数标签
  void VisitFunction(const koopa_raw_function_t &func, std::ostream &out);
  // 访问基本块
  void VisitBasicBlock(const koopa_raw_basic_block_t &bb, std::ostream &out);
  // 访问指令 (按值类型)
  void VisitValue(const koopa_raw_value_t &value, std::ostream &out);
  // 处理全局内存分配 (global alloc)
  void VisitGlobalAlloc(const koopa_raw_value_t &value, std::ostream &out);
  // 处理 return 指令
  void VisitReturn(const koopa_raw_return_t &ret, std::ostream &out);
  // 处理 branch 指令
  void VisitBranch(const koopa_raw_branch_t &branch, std::ostream &out);
  // 处理 jump 指令
  void VisitJump(const koopa_raw_jump_t &jump, std::ostream &out);
  // 处理 call 指令
  void VisitCall(const koopa_raw_call_t &call, const koopa_raw_value_t &value,
                 std::ostream &out);
  // 处理二元指令
  void VisitBinary(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value,
                   std::ostream &out);
  // 处理 alloc 指令
  void VisitAlloc(const koopa_raw_value_t &value);
  // 处理 load 指令
  void VisitLoad(const koopa_raw_load_t &load, const koopa_raw_value_t &value,
                 std::ostream &out);
  // 处理 store 指令
  void VisitStore(const koopa_raw_store_t &store, std::ostream &out);
  // 处理 getelemptr 指令 (数组元素指针计算)
  void VisitGetElemPtr(const koopa_raw_get_elem_ptr_t &gep, const koopa_raw_value_t &value,
                       std::ostream &out);
  // 处理 getptr 指令 (指针运算)
  void VisitGetPtr(const koopa_raw_get_ptr_t &gp, const koopa_raw_value_t &value,
                   std::ostream &out);
  // 递归输出全局变量初始化数据
  void EmitGlobalInit(const koopa_raw_value_t &init, std::ostream &out);
  // 计算 Koopa 类型的字节大小
  int CalcTypeSize(const koopa_raw_type_t &type) const;
  // 读取整数常量
  int32_t VisitInteger(const koopa_raw_integer_t &integer);
  // 输出函数的汇编标签
  void EmitFunctionLabel(const koopa_raw_function_t &func, std::ostream &out);
  // 预处理函数, 计算栈空间 (含 ra/传参) 并分配值的栈偏移
  void PrepareFunction(const koopa_raw_function_t &func);
  // 读取某个值到寄存器
  void LoadValue(const koopa_raw_value_t &value, const std::string &reg,
                 std::ostream &out);
  // 读取浮点值到浮点寄存器
  void LoadFloatValue(const koopa_raw_value_t &value, const std::string &reg,
                      std::ostream &out);
  // 读取地址到寄存器
  void LoadAddress(const koopa_raw_value_t &value, const std::string &reg,
                   std::ostream &out);
  // 将寄存器写回到值对应的栈位置
  void StoreValue(const koopa_raw_value_t &value, const std::string &reg,
                  std::ostream &out);
  // 将浮点寄存器写回到值对应的栈位置
  void StoreFloatValue(const koopa_raw_value_t &value, const std::string &reg,
                       std::ostream &out);
  // 查询 Koopa 值的类型
  ValueType GetValueType(const koopa_raw_value_t &value) const;
  // 判断类型是否为 unit
  bool IsUnitType(const koopa_raw_type_t &type) const;
  // 生成 sp 调整指令
  void EmitAddiSp(int offset, std::ostream &out);
  // 访问栈内偏移的内存 (32-bit word)
  void EmitLoadFromOffset(const std::string &reg, int offset, std::ostream &out);
  void EmitStoreToOffset(const std::string &reg, int offset, std::ostream &out);
  // 访问栈内偏移的内存 (64-bit pointer, RV64)
  void EmitLoadFromOffsetPtr(const std::string &reg, int offset, std::ostream &out);
  void EmitStoreToOffsetPtr(const std::string &reg, int offset, std::ostream &out);
  // 访问栈内偏移的浮点内存
  void EmitLoadFromOffsetFloat(const std::string &reg, int offset, std::ostream &out);
  void EmitStoreToOffsetFloat(const std::string &reg, int offset, std::ostream &out);
  // 判断 Koopa 类型是否为指针类型
  bool IsPointerType(const koopa_raw_type_t &type) const;
  // 生成并输出基本块标签
  std::string FormatBasicBlockLabel(const koopa_raw_basic_block_t &bb) const;
  void EmitBasicBlockLabel(const koopa_raw_basic_block_t &bb, std::ostream &out);

  std::unordered_map<koopa_raw_value_t, int> value_offsets_;
  std::unordered_map<koopa_raw_basic_block_t, std::string> bb_labels_;
  koopa_raw_basic_block_t entry_bb_ = nullptr;
  std::string current_function_name_;
  int stack_size_ = 0;       // 总栈空间 (向上对齐到 16)
  int local_var_size_ = 0;   // 局部变量 + 临时值所占栈空间
  bool has_call_ = false;    // 函数内是否有 call 指令 (决定是否保存 ra)
  int max_call_args_ = 0;    // 最大调用参数个数 (决定栈传参预留空间)
  int param_count_ = 0;      // 函数参数个数 (用于映射参数到 a0-a7)
  ValueType current_return_type_ = ValueType::Int;
  bool data_section_opened_ = false;  // 是否已输出过 .data 段
};
