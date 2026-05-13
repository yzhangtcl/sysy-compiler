#pragma once

#include <iosfwd>

#include "koopa.h"

class AsmGenerator {
 public:
  void Generate(const koopa_raw_program_t &program, std::ostream &out);

 private:
  void VisitSlice(const koopa_raw_slice_t &slice, std::ostream &out);
  void VisitFunction(const koopa_raw_function_t &func, std::ostream &out);
  void VisitBasicBlock(const koopa_raw_basic_block_t &bb, std::ostream &out);
  void VisitValue(const koopa_raw_value_t &value, std::ostream &out);
  void VisitReturn(const koopa_raw_return_t &ret, std::ostream &out);
  int32_t VisitInteger(const koopa_raw_integer_t &integer);
  void EmitFunctionLabel(const koopa_raw_function_t &func, std::ostream &out);
};
