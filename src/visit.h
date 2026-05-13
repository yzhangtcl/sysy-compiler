#pragma once

#include <iosfwd>
#include <string>
#include <unordered_map>

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
  // 访问指令
  void VisitValue(const koopa_raw_value_t &value, std::ostream &out);
  // 处理 return 指令
  void VisitReturn(const koopa_raw_return_t &ret, std::ostream &out);
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
  // 读取整数常量
  int32_t VisitInteger(const koopa_raw_integer_t &integer);
  // 输出函数的汇编标签
  void EmitFunctionLabel(const koopa_raw_function_t &func, std::ostream &out);
  // 预处理函数, 计算栈空间并分配值的栈偏移
  void PrepareFunction(const koopa_raw_function_t &func);
  // 读取某个值到寄存器
  void LoadValue(const koopa_raw_value_t &value, const std::string &reg,
                 std::ostream &out);
  // 读取地址到寄存器
  void LoadAddress(const koopa_raw_value_t &value, const std::string &reg,
                   std::ostream &out);
  // 将寄存器写回到值对应的栈位置
  void StoreValue(const koopa_raw_value_t &value, const std::string &reg,
                  std::ostream &out);
  // 判断类型是否为 unit
  bool IsUnitType(const koopa_raw_type_t &type) const;
  // 生成 sp 调整指令
  void EmitAddiSp(int offset, std::ostream &out);
  // 访问栈内偏移的内存
  void EmitLoadFromOffset(const std::string &reg, int offset, std::ostream &out);
  void EmitStoreToOffset(const std::string &reg, int offset, std::ostream &out);

  std::unordered_map<koopa_raw_value_t, int> value_offsets_;
  int stack_size_ = 0;
};
