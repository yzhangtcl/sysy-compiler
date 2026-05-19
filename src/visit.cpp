#include "visit.h"

#include <cassert>
#include <ostream>

void AsmGenerator::Generate(const koopa_raw_program_t &program, std::ostream &out) {
  // 先生成全局变量 (data 段)
  VisitSlice(program.values, out);
  // 再遍历所有函数并输出汇编
  VisitSlice(program.funcs, out);
}

void AsmGenerator::VisitSlice(const koopa_raw_slice_t &slice, std::ostream &out) {
  // 根据 slice 元素类型分发到具体访问函数
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        VisitFunction(reinterpret_cast<koopa_raw_function_t>(ptr), out);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        VisitBasicBlock(reinterpret_cast<koopa_raw_basic_block_t>(ptr), out);
        break;
      case KOOPA_RSIK_VALUE:
        // 全局值 (global alloc) 也以 VALUE 形式出现
        {
          auto value = reinterpret_cast<koopa_raw_value_t>(ptr);
          if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
            VisitGlobalAlloc(value, out);
          } else {
            VisitValue(value, out);
          }
        }
        break;
      default:
        assert(false);
    }
  }
}

void AsmGenerator::VisitFunction(const koopa_raw_function_t &func, std::ostream &out) {
  // 跳过函数声明 (decl): 无基本块
  if (func->bbs.len == 0) {
    return;
  }

  // 解析函数名 (去掉 @ 前缀)
  const char *raw_name = func->name ? func->name : "@main";
  std::string func_name = raw_name;
  if (!func_name.empty() && func_name[0] == '@') {
    func_name.erase(0, 1);
  }
  current_function_name_ = func_name;
  current_return_type_ = LookupFunctionReturnType(func_name);

  // 恢复该函数的类型表 (前端在生成每个函数时保存了类型信息)
  RestoreValueTypeTable(func_name);

  // 预处理: 计算栈布局
  PrepareFunction(func);

  // 输出函数标签和 prologue
  EmitFunctionLabel(func, out);

  // Prologue: 调整 sp, 必要时保存 ra
  if (stack_size_ > 0) {
    EmitAddiSp(-stack_size_, out);
  }
  // 保存 ra 到栈帧顶部
  if (has_call_) {
    int ra_offset = stack_size_ - 4;
    EmitStoreToOffset("ra", ra_offset, out);
  }

  // 遍历基本块
  VisitSlice(func->bbs, out);
}

void AsmGenerator::VisitBasicBlock(const koopa_raw_basic_block_t &bb, std::ostream &out) {
  // 遍历基本块中的指令
  // 基本块中仅包含指令列表
  EmitBasicBlockLabel(bb, out);
  VisitSlice(bb->insts, out);
}

void AsmGenerator::VisitValue(const koopa_raw_value_t &value, std::ostream &out) {
  // 按 Koopa 指令种类分发生成汇编
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      VisitReturn(kind.data.ret, out);
      break;
    case KOOPA_RVT_BRANCH:
      VisitBranch(kind.data.branch, out);
      break;
    case KOOPA_RVT_JUMP:
      VisitJump(kind.data.jump, out);
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
    case KOOPA_RVT_CALL:
      VisitCall(kind.data.call, value, out);
      break;
    case KOOPA_RVT_INTEGER:
      (void)VisitInteger(kind.data.integer);
      break;
    case KOOPA_RVT_GET_ELEM_PTR:
      VisitGetElemPtr(kind.data.get_elem_ptr, value, out);
      break;
    case KOOPA_RVT_GET_PTR:
      VisitGetPtr(kind.data.get_ptr, value, out);
      break;
    default:
      assert(false);
  }
}

void AsmGenerator::VisitReturn(const koopa_raw_return_t &ret, std::ostream &out) {
  // 处理 return: 计算返回值 (如果有) 并放到 a0/fa0, 然后恢复栈帧
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
      // Float 返回类型
      if (value_type == ValueType::Float) {
        LoadFloatValue(ret.value, "fa0", out);
      } else {
        LoadValue(ret.value, "t0", out);
        out << "  fcvt.s.w fa0, t0\n";
      }
    }
  }

  // Epilogue: 恢复 ra (如果需要), 恢复 sp, ret
  if (has_call_) {
    int ra_offset = stack_size_ - 4;
    EmitLoadFromOffset("ra", ra_offset, out);
  }
  if (stack_size_ > 0) {
    EmitAddiSp(stack_size_, out);
  }
  out << "  ret\n";
}

void AsmGenerator::VisitBranch(const koopa_raw_branch_t &branch, std::ostream &out) {
  // 条件分支: cond != 0 时跳转到真分支, 否则跳转到假分支
  LoadValue(branch.cond, "t0", out);
  auto true_it = bb_labels_.find(branch.true_bb);
  auto false_it = bb_labels_.find(branch.false_bb);
  assert(true_it != bb_labels_.end() && false_it != bb_labels_.end());
  out << "  bnez t0, " << true_it->second << "\n";
  out << "  j " << false_it->second << "\n";
}

void AsmGenerator::VisitJump(const koopa_raw_jump_t &jump, std::ostream &out) {
  // 无条件跳转
  auto target_it = bb_labels_.find(jump.target);
  assert(target_it != bb_labels_.end());
  out << "  j " << target_it->second << "\n";
}

void AsmGenerator::VisitAlloc(const koopa_raw_value_t &value) {
  // alloc 只负责分配栈槽, 不生成指令
  (void)value;
}

void AsmGenerator::VisitLoad(const koopa_raw_load_t &load, const koopa_raw_value_t &value,
                             std::ostream &out) {
  // 读取地址指向的值, 并将结果写回栈
  // 支持全局变量 (global alloc) 和局部变量 (alloc)
  ValueType value_type = GetValueType(value);

  if (value_type == ValueType::Float) {
    if (load.src->kind.tag == KOOPA_RVT_ALLOC) {
      auto it = value_offsets_.find(load.src);
      assert(it != value_offsets_.end());
      EmitLoadFromOffsetFloat("ft0", it->second, out);
    } else if (load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
      LoadAddress(load.src, "t1", out);
      out << "  flw ft0, 0(t1)\n";
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
  } else if (load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    LoadAddress(load.src, "t1", out);
    out << "  lw t0, 0(t1)\n";
  } else {
    LoadAddress(load.src, "t1", out);
    out << "  lw t0, 0(t1)\n";
  }
  StoreValue(value, "t0", out);
}

void AsmGenerator::VisitStore(const koopa_raw_store_t &store, std::ostream &out) {
  // 计算待写入的值, 支持全局变量目标
  ValueType dest_type = GetValueType(store.dest);
  ValueType value_type = GetValueType(store.value);

  if (dest_type == ValueType::Float) {
    // 检查目标 alloc 是否持有指针类型 (数组参数), 如果是则用整数 store
    bool dest_holds_pointer = false;
    if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
      dest_holds_pointer = IsPointerType(store.dest->ty->data.pointer.base);
    }
    if (dest_holds_pointer) {
      // 目标是数组参数 (指针类型): 用整数 store, 不做浮点转换
      LoadValue(store.value, "t0", out);
      if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
        auto it = value_offsets_.find(store.dest);
        assert(it != value_offsets_.end());
        EmitStoreToOffset("t0", it->second, out);
      } else {
        LoadAddress(store.dest, "t1", out);
        out << "  sw t0, 0(t1)\n";
      }
    } else if (value_type == ValueType::Float) {
      LoadFloatValue(store.value, "ft0", out);
      if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
        auto it = value_offsets_.find(store.dest);
        assert(it != value_offsets_.end());
        EmitStoreToOffsetFloat("ft0", it->second, out);
      } else {
        LoadAddress(store.dest, "t1", out);
        out << "  fsw ft0, 0(t1)\n";
      }
    } else if (store.value->kind.tag == KOOPA_RVT_INTEGER) {
      // 整数常量存入 float 标量目标: fcvt.s.w 将整数值转为浮点 (如 float x = 4)
      int32_t imm = VisitInteger(store.value->kind.data.integer);
      out << "  li t0, " << imm << "\n";
      out << "  fcvt.s.w ft0, t0\n";
      if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
        auto it = value_offsets_.find(store.dest);
        assert(it != value_offsets_.end());
        EmitStoreToOffsetFloat("ft0", it->second, out);
      } else {
        LoadAddress(store.dest, "t1", out);
        out << "  fsw ft0, 0(t1)\n";
      }
    } else if (store.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
      // 标量浮点函数参数: 携带 float bit pattern, 用 fmv.w.x
      LoadValue(store.value, "t0", out);
      out << "  fmv.w.x ft0, t0\n";
      if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
        auto it = value_offsets_.find(store.dest);
        assert(it != value_offsets_.end());
        EmitStoreToOffsetFloat("ft0", it->second, out);
      } else {
        LoadAddress(store.dest, "t1", out);
        out << "  fsw ft0, 0(t1)\n";
      }
    } else {
      // 其他整数表达式存入 float 标量目标: fcvt.s.w (如 float x = int_var)
      LoadValue(store.value, "t0", out);
      out << "  fcvt.s.w ft0, t0\n";
      if (store.dest->kind.tag == KOOPA_RVT_ALLOC) {
        auto it = value_offsets_.find(store.dest);
        assert(it != value_offsets_.end());
        EmitStoreToOffsetFloat("ft0", it->second, out);
      } else {
        LoadAddress(store.dest, "t1", out);
        out << "  fsw ft0, 0(t1)\n";
      }
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
  return integer.value;
}

void AsmGenerator::VisitCall(const koopa_raw_call_t &call, const koopa_raw_value_t &value,
                             std::ostream &out) {
  // 处理 call 指令: 将前 8 个参数按被调用函数的 ABI 放入正确寄存器
  const char *callee_name = call.callee->name ? (call.callee->name + 1) : "main";
  std::string callee = callee_name;
  size_t arg_count = call.args.len;

  // 将参数放入寄存器或栈
  for (size_t i = 0; i < arg_count; ++i) {
    auto arg = reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]);
    if (i < 8) {
      // 前 8 个参数
      std::string ireg = "a" + std::to_string(i);
      std::string freg = "fa" + std::to_string(i);

      // putfloat 唯一参数必须走 fa0 (即使实参是 int, 也需 fcvt.s.w 转换)
      if (callee == "putfloat") {
        LoadFloatValue(arg, freg, out);
      }
      // putint 如果实参是 float, 需要 fcvt.w.s 转整数再放 a0
      else if (callee == "putint" && GetValueType(arg) == ValueType::Float) {
        LoadFloatValue(arg, "ft0", out);
        out << "  fcvt.w.s " << ireg << ", ft0, rtz\n";
      }
      // 默认: 整型传参
      else {
        LoadValue(arg, ireg, out);
      }
    } else {
      // 超出 8 个参数: 放入栈帧 (sp + (i - 8) * 4)
      LoadValue(arg, "t0", out);
      int offset = static_cast<int>(i - 8) * 4;
      EmitStoreToOffset("t0", offset, out);
    }
  }

  // 执行 call 指令
  out << "  call " << callee_name << "\n";

  // 将返回值 (如果有) 写回栈: float 返回值在 fa0, 其他在 a0
  if (!IsUnitType(value->ty)) {
    if (GetValueType(value) == ValueType::Float) {
      StoreFloatValue(value, "fa0", out);
    } else {
      StoreValue(value, "a0", out);
    }
  }
}

void AsmGenerator::VisitGetElemPtr(const koopa_raw_get_elem_ptr_t &gep,
                                   const koopa_raw_value_t &value, std::ostream &out) {
  // getelemptr src, index: 计算 src + index * sizeof(element_type)
  // src 类型为 *[T, N], 结果类型为 *T
  // 加载基地址
  LoadAddress(gep.src, "t0", out);
  // 加载索引
  LoadValue(gep.index, "t1", out);
  // 计算元素大小
  int elem_size = CalcTypeSize(gep.src->ty->data.pointer.base->data.array.base);
  // t1 = index * elem_size
  out << "  li t2, " << elem_size << "\n";
  out << "  mul t1, t1, t2\n";
  // t0 = base + offset
  out << "  add t0, t0, t1\n";
  // 存储结果地址
  StoreValue(value, "t0", out);
}

void AsmGenerator::VisitGetPtr(const koopa_raw_get_ptr_t &gp,
                               const koopa_raw_value_t &value, std::ostream &out) {
  // getptr src, index: 计算 src + index * sizeof(pointed_type)
  // src 类型为 *T, 结果类型也为 *T
  // 加载基地址
  LoadAddress(gp.src, "t0", out);
  // 加载索引
  LoadValue(gp.index, "t1", out);
  // 计算指向类型的大小
  int elem_size = CalcTypeSize(gp.src->ty->data.pointer.base);
  // t1 = index * elem_size
  out << "  li t2, " << elem_size << "\n";
  out << "  mul t1, t1, t2\n";
  // t0 = base + offset
  out << "  add t0, t0, t1\n";
  // 存储结果地址
  StoreValue(value, "t0", out);
}

void AsmGenerator::VisitGlobalAlloc(const koopa_raw_value_t &value, std::ostream &out) {
  // 处理全局内存分配: 输出 .data 段和符号定义
  if (!data_section_opened_) {
    out << "  .data\n";
    data_section_opened_ = true;
  }

  // 解析全局变量名 (去掉 @ 前缀)
  const char *raw_name = value->name ? value->name : "@global";
  std::string name = raw_name;
  if (!name.empty() && name[0] == '@') {
    name.erase(0, 1);
  }

  out << "  .globl " << name << "\n";
  out << name << ":\n";

  // 根据初始化器类型输出
  auto &kind = value->kind;
  if (kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    auto init = kind.data.global_alloc.init;
    EmitGlobalInit(init, out);
  }
}

void AsmGenerator::EmitGlobalInit(const koopa_raw_value_t &init, std::ostream &out) {
  // 递归输出全局变量初始化数据
  if (init->kind.tag == KOOPA_RVT_ZERO_INIT) {
    // 零初始化: 计算类型大小并输出对应字节数
    int size = CalcTypeSize(init->ty);
    out << "  .zero " << size << "\n";
  } else if (init->kind.tag == KOOPA_RVT_INTEGER) {
    int32_t val = init->kind.data.integer.value;
    out << "  .word " << val << "\n";
  } else if (init->kind.tag == KOOPA_RVT_AGGREGATE) {
    // 聚合常量: 递归输出每个元素
    auto &agg = init->kind.data.aggregate;
    for (size_t i = 0; i < agg.elems.len; ++i) {
      auto elem = reinterpret_cast<koopa_raw_value_t>(agg.elems.buffer[i]);
      EmitGlobalInit(elem, out);
    }
  } else {
    // 未知类型, 默认填 0
    out << "  .zero 4\n";
  }
}

int AsmGenerator::CalcTypeSize(const koopa_raw_type_t &type) const {
  // 计算 Koopa 类型的字节大小
  if (type->tag == KOOPA_RTT_INT32) {
    return 4;
  } else if (type->tag == KOOPA_RTT_ARRAY) {
    return type->data.array.len * CalcTypeSize(type->data.array.base);
  } else if (type->tag == KOOPA_RTT_POINTER) {
    return 4;  // 指针大小为 4 字节 (32 位模式)
  } else if (type->tag == KOOPA_RTT_UNIT) {
    return 0;
  }
  return 4;  // 默认
}

void AsmGenerator::EmitFunctionLabel(const koopa_raw_function_t &func, std::ostream &out) {
  // 输出函数标签及其全局符号声明
  const char *name = func->name ? func->name : "@main";
  if (name[0] == '@') {
    ++name;
  }
  // 总是输出 .text 确保从 .data 切换回来
  out << "  .text\n";
  out << "  .globl " << name << "\n";
  out << name << ":\n";
}

void AsmGenerator::PrepareFunction(const koopa_raw_function_t &func) {
  // 扫描函数内所有指令:
  // 1. 为需要落栈的值分配栈偏移
  // 2. 检测是否有 call 指令 (决定是否保存 ra)
  // 3. 统计最大调用参数个数 (决定栈传参预留空间)
  value_offsets_.clear();
  bb_labels_.clear();
  entry_bb_ = nullptr;
  has_call_ = false;
  max_call_args_ = 0;
  local_var_size_ = 0;
  stack_size_ = 0;
  // 函数参数个数 (从 func->params 获取)
  param_count_ = static_cast<int>(func->params.len);

  auto bbs = func->bbs;
  for (size_t i = 0; i < bbs.len; ++i) {
    auto bb = reinterpret_cast<koopa_raw_basic_block_t>(bbs.buffer[i]);
    if (i == 0) {
      entry_bb_ = bb;
    }
    bb_labels_[bb] = FormatBasicBlockLabel(bb);
    auto insts = bb->insts;
    for (size_t j = 0; j < insts.len; ++j) {
      auto value = reinterpret_cast<koopa_raw_value_t>(insts.buffer[j]);

      // 检测 call 指令
      if (value->kind.tag == KOOPA_RVT_CALL) {
        has_call_ = true;
        auto &call_data = value->kind.data.call;
        // 统计参数个数
        size_t arg_count = call_data.args.len;
        if (static_cast<int>(arg_count) > max_call_args_) {
          max_call_args_ = static_cast<int>(arg_count);
        }
      }

      // 为有返回值的指令分配栈槽
      if (!IsUnitType(value->ty)) {
        value_offsets_[value] = local_var_size_;
        int size;
        if (value->kind.tag == KOOPA_RVT_ALLOC) {
          // alloc 指令: 为指向的类型分配空间 (如 alloc [i32, 5] 需要 20 字节)
          size = CalcTypeSize(value->ty->data.pointer.base);
        } else {
          size = CalcTypeSize(value->ty);
        }
        local_var_size_ += size;
      }
    }
  }

  // 计算总栈空间: 局部变量 + ra + 超 8 参数传参空间
  // ra 占用 4 字节 (仅非叶子函数)
  int ra_size = has_call_ ? 4 : 0;
  // 超出 8 个参数的调用传参空间, 每个参数 4 字节
  int arg_stack = (max_call_args_ > 8) ? (max_call_args_ - 8) * 4 : 0;
  stack_size_ = local_var_size_ + ra_size + arg_stack;

  // 向上对齐到 16 字节
  if (stack_size_ % 16 != 0) {
    stack_size_ = (stack_size_ + 15) / 16 * 16;
  }

  // 重新计算 ra 和局部变量在栈帧中的偏移:
  // 栈帧布局 (从高地址到低地址):
  //   sp + stack_size_ - 4: ra (如果保存)
  //   sp + stack_size_ - 4 - ra_size: 局部变量区域
  //   参数传参区域在栈顶部 (高地址), 但这里我们只需要在 call 时用 sp+offset 写入
  //   所以局部变量的偏移需要调整

  // 调整偏移: 传参区域在局部变量之上
  int base = arg_stack;
  for (auto &kv : value_offsets_) {
    kv.second += base;
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
  // 函数参数引用: 从 a0-a7 中读取 (参数在函数入口时位于寄存器)
  if (value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
    int param_index = value->kind.data.func_arg_ref.index;
    if (param_index < 8) {
      out << "  mv " << reg << ", a" << param_index << "\n";
    } else {
      // 超出 8 个参数在栈上 (sp + (param_index - 8) * 4 + offset_of_sp_adjustment)
      int offset = (param_index - 8) * 4 + stack_size_;
      EmitLoadFromOffset(reg, offset, out);
    }
    return;
  }
  auto it = value_offsets_.find(value);
  if (it != value_offsets_.end()) {
    EmitLoadFromOffset(reg, it->second, out);
    return;
  }
  // 回退: 用 LoadAddress + lw 加载 (处理全局变量等未在栈中分配的值)
  LoadAddress(value, "t1", out);
  out << "  lw " << reg << ", 0(t1)\n";
}

void AsmGenerator::LoadFloatValue(const koopa_raw_value_t &value, const std::string &reg,
                                  std::ostream &out) {
  // 将值加载到浮点寄存器
  // 注意: Koopa IR 中 float 以 i32 bit pattern 形式承载,
  // 因此 INTEGER 常量是 bit pattern, 需要用 fmv.w.x 而非 fcvt.s.w
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    int32_t imm = VisitInteger(value->kind.data.integer);
    out << "  li t0, " << imm << "\n";
    out << "  fmv.w.x " << reg << ", t0\n";
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
  // 全局变量: 使用 la 伪指令
  if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    const char *name = value->name ? value->name : "@global";
    if (name[0] == '@') {
      ++name;
    }
    out << "  la " << reg << ", " << name << "\n";
    return;
  }
  // alloc 指令: 返回 sp + offset (alloc 本身就在栈上分配空间)
  if (value->kind.tag == KOOPA_RVT_ALLOC) {
    auto it = value_offsets_.find(value);
    assert(it != value_offsets_.end());
    int offset = it->second;
    if (offset >= -2048 && offset <= 2047) {
      out << "  addi " << reg << ", sp, " << offset << "\n";
    } else {
      out << "  li " << reg << ", " << offset << "\n";
      out << "  add " << reg << ", sp, " << reg << "\n";
    }
    return;
  }
  // getelemptr / getptr / 其他: 结果地址存储在栈槽中, 需要加载
  auto it = value_offsets_.find(value);
  assert(it != value_offsets_.end());
  EmitLoadFromOffset(reg, it->second, out);
}

void AsmGenerator::StoreValue(const koopa_raw_value_t &value, const std::string &reg,
                              std::ostream &out) {
  // 将寄存器中的结果写回值对应的栈槽
  auto it = value_offsets_.find(value);
  assert(it != value_offsets_.end());
  if (IsPointerType(value->ty)) {
    EmitStoreToOffsetPtr(reg, it->second, out);
  } else {
    EmitStoreToOffset(reg, it->second, out);
  }
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

bool AsmGenerator::IsPointerType(const koopa_raw_type_t &type) const {
  return type->tag == KOOPA_RTT_POINTER;
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
  // 向 sp + offset 写入数据 (32-bit word), 处理 12 位偏移限制
  if (offset >= -2048 && offset <= 2047) {
    out << "  sw " << reg << ", " << offset << "(sp)\n";
  } else {
    out << "  li t2, " << offset << "\n";
    out << "  add t2, sp, t2\n";
    out << "  sw " << reg << ", 0(t2)\n";
  }
}

void AsmGenerator::EmitStoreToOffsetPtr(const std::string &reg, int offset,
                                        std::ostream &out) {
  // 向 sp + offset 写入指针值 (RV32 使用 sw, RV64 需要 sd, 此处统一用 sw)
  EmitStoreToOffset(reg, offset, out);
}

void AsmGenerator::EmitLoadFromOffsetPtr(const std::string &reg, int offset,
                                         std::ostream &out) {
  // 从 sp + offset 加载指针值 (RV32 使用 lw, RV64 需要 ld, 此处统一用 lw)
  EmitLoadFromOffset(reg, offset, out);
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

std::string AsmGenerator::FormatBasicBlockLabel(const koopa_raw_basic_block_t &bb) const {
  // 基本块名去掉 % 前缀, 并加上函数前缀确保唯一
  const char *raw_name = bb->name ? bb->name : "%bb";
  std::string name = raw_name;
  if (!name.empty() && name[0] == '%') {
    name.erase(0, 1);
  }
  if (current_function_name_.empty()) {
    return name;
  }
  return current_function_name_ + "_" + name;
}

void AsmGenerator::EmitBasicBlockLabel(const koopa_raw_basic_block_t &bb, std::ostream &out) {
  // 输出基本块标签
  auto it = bb_labels_.find(bb);
  if (it == bb_labels_.end()) {
    return;
  }
  out << it->second << ":\n";
}
