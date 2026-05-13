#include "visit.h"

#include <cassert>
#include <ostream>

void AsmGenerator::Generate(const koopa_raw_program_t &program, std::ostream &out) {
  // 仅遍历函数列表
  VisitSlice(program.funcs, out);
}

void AsmGenerator::VisitSlice(const koopa_raw_slice_t &slice, std::ostream &out) {
  for (size_t i = 0; i < slice.len; ++i) {
    // raw slice 的元素是 void* 指针, 需要按 kind 解释
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        VisitFunction(reinterpret_cast<koopa_raw_function_t>(ptr), out);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        VisitBasicBlock(reinterpret_cast<koopa_raw_basic_block_t>(ptr), out);
        break;
      case KOOPA_RSIK_VALUE:
        VisitValue(reinterpret_cast<koopa_raw_value_t>(ptr), out);
        break;
      default:
        assert(false);
    }
  }
}

void AsmGenerator::VisitFunction(const koopa_raw_function_t &func, std::ostream &out) {
  // 先输出函数标签再遍历基本块
  EmitFunctionLabel(func, out);
  VisitSlice(func->bbs, out);
}

void AsmGenerator::VisitBasicBlock(const koopa_raw_basic_block_t &bb, std::ostream &out) {
  // 基本块中仅包含指令列表
  VisitSlice(bb->insts, out);
}

void AsmGenerator::VisitValue(const koopa_raw_value_t &value, std::ostream &out) {
  // 按指令类型分发
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      VisitReturn(kind.data.ret, out);
      break;
    case KOOPA_RVT_INTEGER:
      (void)VisitInteger(kind.data.integer);
      break;
    default:
      assert(false);
  }
}

void AsmGenerator::VisitReturn(const koopa_raw_return_t &ret, std::ostream &out) {
  // 当前只有整数返回
  if (ret.value != nullptr) {
    assert(ret.value->kind.tag == KOOPA_RVT_INTEGER);
    int32_t imm = VisitInteger(ret.value->kind.data.integer);
    // 将返回值放入 a0
    out << "  li a0, " << imm << "\n";
  }
  // 函数返回
  out << "  ret\n";
}

int32_t AsmGenerator::VisitInteger(const koopa_raw_integer_t &integer) {
  // 读取整数常量数值
  return integer.value;
}

void AsmGenerator::EmitFunctionLabel(const koopa_raw_function_t &func, std::ostream &out) {
  // Koopa 函数名以 @ 开头, 汇编标签不带 @
  const char *name = func->name ? func->name : "@main";
  if (name[0] == '@') {
    ++name;
  }
  // 输出段声明与全局符号
  out << "  .text\n";
  out << "  .globl " << name << "\n";
  out << name << ":\n";
}
