#pragma once

#include <iosfwd>

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
  // 读取整数常量
  int32_t VisitInteger(const koopa_raw_integer_t &integer);
  // 输出函数的汇编标签
  void EmitFunctionLabel(const koopa_raw_function_t &func, std::ostream &out);
};
