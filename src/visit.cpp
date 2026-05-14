#include "visit.h"

#include <cassert>
#include <ostream>

void AsmGenerator::Generate(const koopa_raw_program_t &program, std::ostream &out) {
  // 遍历 raw program 中的所有函数并输出汇编
  // 仅遍历函数列表
  VisitSlice(program.funcs, out);
}

void AsmGenerator::VisitSlice(const koopa_raw_slice_t &slice, std::ostream &out) {
  // 根据 slice 元素类型分发到具体访问函数
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
  // 预处理函数并生成函数级汇编骨架
  // 先输出函数标签再遍历基本块
  PrepareFunction(func);
  const char *raw_name = func->name ? func->name : "@main";
  std::string func_name = raw_name;
  if (!func_name.empty() && func_name[0] == '@') {
    func_name.erase(0, 1);
  }
  current_return_type_ = LookupFunctionReturnType(func_name);
  EmitFunctionLabel(func, out);
  if (stack_size_ > 0) {
    EmitAddiSp(-stack_size_, out);
  }
  VisitSlice(func->bbs, out);
}

void AsmGenerator::VisitBasicBlock(const koopa_raw_basic_block_t &bb, std::ostream &out) {
  // 遍历基本块中的指令
  // 基本块中仅包含指令列表
  VisitSlice(bb->insts, out);
}

void AsmGenerator::VisitValue(const koopa_raw_value_t &value, std::ostream &out) {
  // 按 Koopa 指令种类分发生成汇编
  // 按指令类型分发
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      VisitReturn(kind.data.ret, out);
      break;
    case KOOPA_RVT_ALLOC:
      VisitAlloc(value);
      break;
    case KOOPA_RVT_LOAD:
      VisitLoad(kind.data.load, value, out);
      break;
    case KOOPA_RVT_STORE:
      VisitStore(kind.data.store, out);
      break;
    case KOOPA_RVT_BINARY:
      VisitBinary(kind.data.binary, value, out);
      break;
    case KOOPA_RVT_INTEGER:
      (void)VisitInteger(kind.data.integer);
      break;
    default:
      assert(false);
  }
}

void AsmGenerator::VisitReturn(const koopa_raw_return_t &ret, std::ostream &out) {
  // 处理 return: 计算返回值并恢复栈帧
  // 返回值按函数返回类型处理
  if (ret.value != nullptr) {
    ValueType value_type = GetValueType(ret.value);
    if (current_return_type_ == ValueType::Int) {
      if (value_type == ValueType::Float) {
        LoadFloatValue(ret.value, "ft0", out);
        out << "  fcvt.w.s a0, ft0, rtz\n";
      } else if (ret.value->kind.tag == KOOPA_RVT_INTEGER) {
        int32_t imm = VisitInteger(ret.value->kind.data.integer);
        out << "  li a0, " << imm << "\n";
      } else {
        LoadValue(ret.value, "a0", out);
      }
    } else {
      if (value_type == ValueType::Float) {
        LoadFloatValue(ret.value, "fa0", out);
      } else {
        LoadValue(ret.value, "t0", out);
        out << "  fcvt.s.w fa0, t0\n";
      }
    }
  }
  if (stack_size_ > 0) {
    EmitAddiSp(stack_size_, out);
  }
  // 函数返回
  out << "  ret\n";
}

void AsmGenerator::VisitAlloc(const koopa_raw_value_t &value) {
  // alloc 只负责分配栈槽, 不生成指令
  (void)value;
}

void AsmGenerator::VisitLoad(const koopa_raw_load_t &load, const koopa_raw_value_t &value,
                             std::ostream &out) {
  // 读取地址指向的值, 并将结果写回栈
  ValueType value_type = GetValueType(value);
  if (value_type == ValueType::Float) {
    if (load.src->kind.tag == KOOPA_RVT_ALLOC) {
      auto it = value_offsets_.find(load.src);
      assert(it != value_offsets_.end());
      EmitLoadFromOffsetFloat("ft0", it->second, out);
    } else {
      LoadAddress(load.src, "t1", out);
      out << "  flw ft0, 0(t1)\n";
    }
    StoreFloatValue(value, "ft0", out);
    return;
  }

  if (load.src->kind.tag == KOOPA_RVT_ALLOC) {
    auto it = value_offsets_.find(load.src);
    assert(it != value_offsets_.end());
    EmitLoadFromOffset("t0", it->second, out);
  } else {
    LoadAddress(load.src, "t1", out);
    out << "  lw t0, 0(t1)\n";
  }
  StoreValue(value, "t0", out);
}

void AsmGenerator::VisitStore(const koopa_raw_store_t &store, std::ostream &out) {
  // 计算待写入的值
  ValueType dest_type = GetValueType(store.dest);
  ValueType value_type = GetValueType(store.value);
  if (dest_type == ValueType::Float) {
    if (value_type == ValueType::Float) {
      LoadFloatValue(store.value, "ft0", out);
    } else if (store.value->kind.tag == KOOPA_RVT_INTEGER) {
      int32_t imm = VisitInteger(store.value->kind.data.integer);
      out << "  li t0, " << imm << "\n";
      out << "  fcvt.s.w ft0, t0\n";
    } else {
      LoadValue(store.value, "t0", out);
      out << "  fcvt.s.w ft0, t0\n";
    }
    if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
      auto it = value_offsets_.find(store.dest);
      assert(it != value_offsets_.end());
      EmitStoreToOffsetFloat("ft0", it->second, out);
    } else {
      LoadAddress(store.dest, "t1", out);
      out << "  fsw ft0, 0(t1)\n";
    }
    return;
  }

  if (value_type == ValueType::Float) {
    LoadFloatValue(store.value, "ft0", out);
    out << "  fcvt.w.s t0, ft0, rtz\n";
  } else {
    LoadValue(store.value, "t0", out);
  }
  if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
    auto it = value_offsets_.find(store.dest);
    assert(it != value_offsets_.end());
    EmitStoreToOffset("t0", it->second, out);
  } else {
    LoadAddress(store.dest, "t1", out);
    out << "  sw t0, 0(t1)\n";
  }
}

void AsmGenerator::VisitBinary(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value,
                               std::ostream &out) {
  // 处理二元运算: 加载两侧操作数, 计算并写回栈
  ValueType lhs_type = GetValueType(binary.lhs);
  ValueType rhs_type = GetValueType(binary.rhs);
  ValueType result_type = GetValueType(value);
  bool use_float = lhs_type == ValueType::Float || rhs_type == ValueType::Float ||
                   result_type == ValueType::Float;

  if (use_float) {
    if (result_type == ValueType::Float && binary.op == KOOPA_RBO_ADD &&
        binary.lhs->kind.tag == KOOPA_RVT_INTEGER &&
        binary.rhs->kind.tag == KOOPA_RVT_INTEGER &&
        VisitInteger(binary.lhs->kind.data.integer) == 0) {
      int32_t bits = VisitInteger(binary.rhs->kind.data.integer);
      out << "  li t0, " << bits << "\n";
      out << "  fmv.w.x ft0, t0\n";
      StoreFloatValue(value, "ft0", out);
      return;
    }

    auto load_float_operand = [&](const koopa_raw_value_t &operand, ValueType type,
                                  const std::string &freg) {
      if (type == ValueType::Float) {
        if (operand->kind.tag == KOOPA_RVT_INTEGER) {
          int32_t bits = VisitInteger(operand->kind.data.integer);
          out << "  li t0, " << bits << "\n";
          out << "  fmv.w.x " << freg << ", t0\n";
          return;
        }
        auto it = value_offsets_.find(operand);
        assert(it != value_offsets_.end());
        EmitLoadFromOffsetFloat(freg, it->second, out);
        return;
      }
      LoadValue(operand, "t0", out);
      out << "  fcvt.s.w " << freg << ", t0\n";
    };

    switch (binary.op) {
      case KOOPA_RBO_ADD:
      case KOOPA_RBO_SUB:
      case KOOPA_RBO_MUL:
      case KOOPA_RBO_DIV: {
        load_float_operand(binary.lhs, lhs_type, "ft0");
        load_float_operand(binary.rhs, rhs_type, "ft1");
        switch (binary.op) {
          case KOOPA_RBO_ADD:
            out << "  fadd.s ft0, ft0, ft1\n";
            break;
          case KOOPA_RBO_SUB:
            out << "  fsub.s ft0, ft0, ft1\n";
            break;
          case KOOPA_RBO_MUL:
            out << "  fmul.s ft0, ft0, ft1\n";
            break;
          case KOOPA_RBO_DIV:
            out << "  fdiv.s ft0, ft0, ft1\n";
            break;
          default:
            break;
        }
        StoreFloatValue(value, "ft0", out);
        return;
      }
      case KOOPA_RBO_EQ:
      case KOOPA_RBO_NOT_EQ:
      case KOOPA_RBO_LT:
      case KOOPA_RBO_GT:
      case KOOPA_RBO_LE:
      case KOOPA_RBO_GE: {
        load_float_operand(binary.lhs, lhs_type, "ft0");
        load_float_operand(binary.rhs, rhs_type, "ft1");
        switch (binary.op) {
          case KOOPA_RBO_EQ:
            out << "  feq.s t0, ft0, ft1\n";
            break;
          case KOOPA_RBO_NOT_EQ:
            out << "  feq.s t0, ft0, ft1\n";
            out << "  seqz t0, t0\n";
            break;
          case KOOPA_RBO_LT:
            out << "  flt.s t0, ft0, ft1\n";
            break;
          case KOOPA_RBO_GT:
            out << "  flt.s t0, ft1, ft0\n";
            break;
          case KOOPA_RBO_LE:
            out << "  fle.s t0, ft0, ft1\n";
            break;
          case KOOPA_RBO_GE:
            out << "  fle.s t0, ft1, ft0\n";
            break;
          default:
            break;
        }
        StoreValue(value, "t0", out);
        return;
      }
      default:
        assert(false);
    }
  }

  LoadValue(binary.lhs, "t0", out);
  LoadValue(binary.rhs, "t1", out);

  switch (binary.op) {
    case KOOPA_RBO_ADD:
      out << "  add t0, t0, t1\n";
      break;
    case KOOPA_RBO_SUB:
      out << "  sub t0, t0, t1\n";
      break;
    case KOOPA_RBO_MUL:
      out << "  mul t0, t0, t1\n";
      break;
    case KOOPA_RBO_DIV:
      out << "  div t0, t0, t1\n";
      break;
    case KOOPA_RBO_MOD:
      out << "  rem t0, t0, t1\n";
      break;
    case KOOPA_RBO_AND:
      out << "  and t0, t0, t1\n";
      break;
    case KOOPA_RBO_OR:
      out << "  or t0, t0, t1\n";
      break;
    case KOOPA_RBO_XOR:
      out << "  xor t0, t0, t1\n";
      break;
    case KOOPA_RBO_EQ:
      out << "  xor t0, t0, t1\n";
      out << "  seqz t0, t0\n";
      break;
    case KOOPA_RBO_NOT_EQ:
      out << "  xor t0, t0, t1\n";
      out << "  snez t0, t0\n";
      break;
    case KOOPA_RBO_LT:
      out << "  slt t0, t0, t1\n";
      break;
    case KOOPA_RBO_GT:
      out << "  sgt t0, t0, t1\n";
      break;
    case KOOPA_RBO_LE:
      out << "  sgt t0, t0, t1\n";
      out << "  seqz t0, t0\n";
      break;
    case KOOPA_RBO_GE:
      out << "  slt t0, t0, t1\n";
      out << "  seqz t0, t0\n";
      break;
    default:
      assert(false);
  }

  StoreValue(value, "t0", out);
}

int32_t AsmGenerator::VisitInteger(const koopa_raw_integer_t &integer) {
  // 读取整数常量值
  // 读取整数常量数值
  return integer.value;
}

void AsmGenerator::EmitFunctionLabel(const koopa_raw_function_t &func, std::ostream &out) {
  // 输出函数标签及其全局符号声明
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

void AsmGenerator::PrepareFunction(const koopa_raw_function_t &func) {
  // 扫描函数内指令, 为需要落栈的值分配栈偏移
  value_offsets_.clear();
  stack_size_ = 0;
  auto bbs = func->bbs;
  for (size_t i = 0; i < bbs.len; ++i) {
    auto bb = reinterpret_cast<koopa_raw_basic_block_t>(bbs.buffer[i]);
    auto insts = bb->insts;
    for (size_t j = 0; j < insts.len; ++j) {
      auto value = reinterpret_cast<koopa_raw_value_t>(insts.buffer[j]);
      if (!IsUnitType(value->ty)) {
        value_offsets_[value] = stack_size_;
        stack_size_ += 4;
      }
    }
  }
  if (stack_size_ % 16 != 0) {
    stack_size_ = (stack_size_ + 15) / 16 * 16;
  }
}

ValueType AsmGenerator::GetValueType(const koopa_raw_value_t &value) const {
  if (value->name != nullptr) {
    ValueType type = LookupValueType(value->name);
    if (type == ValueType::Float) {
      return type;
    }
    if (value->name[0] == '%' && value->name[1] == 'f') {
      return ValueType::Float;
    }
    return type;
  }
  return ValueType::Int;
}

void AsmGenerator::LoadValue(const koopa_raw_value_t &value, const std::string &reg,
                             std::ostream &out) {
  // 将值加载到目标寄存器, 立即数直接用 li
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    int32_t imm = VisitInteger(value->kind.data.integer);
    out << "  li " << reg << ", " << imm << "\n";
    return;
  }
  auto it = value_offsets_.find(value);
  assert(it != value_offsets_.end());
  EmitLoadFromOffset(reg, it->second, out);
}

void AsmGenerator::LoadFloatValue(const koopa_raw_value_t &value, const std::string &reg,
                                  std::ostream &out) {
  // 将值加载到浮点寄存器, 整数先转为浮点
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    int32_t imm = VisitInteger(value->kind.data.integer);
    out << "  li t0, " << imm << "\n";
    out << "  fcvt.s.w " << reg << ", t0\n";
    return;
  }
  ValueType type = GetValueType(value);
  if (type == ValueType::Float) {
    auto it = value_offsets_.find(value);
    assert(it != value_offsets_.end());
    EmitLoadFromOffsetFloat(reg, it->second, out);
    return;
  }
  LoadValue(value, "t0", out);
  out << "  fcvt.s.w " << reg << ", t0\n";
}

void AsmGenerator::LoadAddress(const koopa_raw_value_t &value, const std::string &reg,
                               std::ostream &out) {
  // 将地址类型的值加载到寄存器
  auto it = value_offsets_.find(value);
  assert(it != value_offsets_.end());
  int offset = it->second;
  if (offset >= -2048 && offset <= 2047) {
    out << "  addi " << reg << ", sp, " << offset << "\n";
  } else {
    out << "  li " << reg << ", " << offset << "\n";
    out << "  add " << reg << ", sp, " << reg << "\n";
  }
}

void AsmGenerator::StoreValue(const koopa_raw_value_t &value, const std::string &reg,
                              std::ostream &out) {
  // 将寄存器中的结果写回值对应的栈槽
  auto it = value_offsets_.find(value);
  assert(it != value_offsets_.end());
  EmitStoreToOffset(reg, it->second, out);
}

void AsmGenerator::StoreFloatValue(const koopa_raw_value_t &value, const std::string &reg,
                                   std::ostream &out) {
  // 将浮点寄存器写回值对应的栈槽
  auto it = value_offsets_.find(value);
  assert(it != value_offsets_.end());
  EmitStoreToOffsetFloat(reg, it->second, out);
}

bool AsmGenerator::IsUnitType(const koopa_raw_type_t &type) const {
  return type->tag == KOOPA_RTT_UNIT;
}

void AsmGenerator::EmitAddiSp(int offset, std::ostream &out) {
  // 调整 sp, 处理 12 位立即数范围
  if (offset >= -2048 && offset <= 2047) {
    out << "  addi sp, sp, " << offset << "\n";
  } else {
    out << "  li t0, " << offset << "\n";
    out << "  add sp, sp, t0\n";
  }
}

void AsmGenerator::EmitLoadFromOffset(const std::string &reg, int offset,
                                      std::ostream &out) {
  // 从 sp + offset 加载数据, 处理 12 位偏移限制
  if (offset >= -2048 && offset <= 2047) {
    out << "  lw " << reg << ", " << offset << "(sp)\n";
  } else {
    out << "  li t2, " << offset << "\n";
    out << "  add t2, sp, t2\n";
    out << "  lw " << reg << ", 0(t2)\n";
  }
}

void AsmGenerator::EmitStoreToOffset(const std::string &reg, int offset,
                                     std::ostream &out) {
  // 向 sp + offset 写入数据, 处理 12 位偏移限制
  if (offset >= -2048 && offset <= 2047) {
    out << "  sw " << reg << ", " << offset << "(sp)\n";
  } else {
    out << "  li t2, " << offset << "\n";
    out << "  add t2, sp, t2\n";
    out << "  sw " << reg << ", 0(t2)\n";
  }
}

void AsmGenerator::EmitLoadFromOffsetFloat(const std::string &reg, int offset,
                                           std::ostream &out) {
  // 从 sp + offset 加载浮点数据
  if (offset >= -2048 && offset <= 2047) {
    out << "  flw " << reg << ", " << offset << "(sp)\n";
  } else {
    out << "  li t2, " << offset << "\n";
    out << "  add t2, sp, t2\n";
    out << "  flw " << reg << ", 0(t2)\n";
  }
}

void AsmGenerator::EmitStoreToOffsetFloat(const std::string &reg, int offset,
                                          std::ostream &out) {
  // 向 sp + offset 写入浮点数据
  if (offset >= -2048 && offset <= 2047) {
    out << "  fsw " << reg << ", " << offset << "(sp)\n";
  } else {
    out << "  li t2, " << offset << "\n";
    out << "  add t2, sp, t2\n";
    out << "  fsw " << reg << ", 0(t2)\n";
  }
}
