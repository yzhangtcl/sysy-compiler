  .text
  .globl f1d
f1d:
  addi sp, sp, -48
f1d_entry:
  mv t0, a0
  sw t0, 0(sp)
  li t0, 0
  sw t0, 4(sp)
  j f1d_while_entry_0
f1d_while_entry_0:
  lw t0, 4(sp)
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 10
  slt t0, t0, t1
  sw t0, 12(sp)
  lw t0, 12(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 16(sp)
  lw t0, 16(sp)
  bnez t0, f1d_while_body_1
  j f1d_while_end_2
f1d_while_body_1:
  lw t0, 4(sp)
  sw t0, 20(sp)
  flw ft0, 0(sp)
  fsw ft0, 24(sp)
  lw t0, 4(sp)
  sw t0, 28(sp)
  lw t0, 24(sp)
  lw t1, 28(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 32(sp)
  lw t0, 20(sp)
  fcvt.s.w ft0, t0
  lw t1, 32(sp)
  fsw ft0, 0(t1)
  lw t0, 4(sp)
  sw t0, 36(sp)
  lw t0, 36(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 40(sp)
  lw t0, 40(sp)
  sw t0, 4(sp)
  j f1d_while_entry_0
f1d_while_end_2:
  addi sp, sp, 48
  ret
  .text
  .globl f2d
f2d:
  addi sp, sp, -64
  sw ra, 60(sp)
f2d_entry:
  mv t0, a0
  sw t0, 0(sp)
  flw ft0, 0(sp)
  fsw ft0, 4(sp)
  lw t0, 4(sp)
  li t1, 1
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 12(sp)
  li t0, 3
  fcvt.s.w ft0, t0
  lw t1, 12(sp)
  fsw ft0, 0(t1)
  li t0, 0
  sw t0, 16(sp)
  j f2d_while_entry_0
f2d_while_entry_0:
  lw t0, 16(sp)
  sw t0, 20(sp)
  lw t0, 20(sp)
  li t1, 10
  slt t0, t0, t1
  sw t0, 24(sp)
  lw t0, 24(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 28(sp)
  lw t0, 28(sp)
  bnez t0, f2d_while_body_1
  j f2d_while_end_2
f2d_while_body_1:
  flw ft0, 0(sp)
  fsw ft0, 32(sp)
  lw t0, 16(sp)
  sw t0, 36(sp)
  lw t0, 32(sp)
  lw t1, 36(sp)
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 40(sp)
  lw t0, 40(sp)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 44(sp)
  lw a0, 44(sp)
  call f1d
  lw t0, 16(sp)
  sw t0, 48(sp)
  lw t0, 48(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 52(sp)
  lw t0, 52(sp)
  sw t0, 16(sp)
  j f2d_while_entry_0
f2d_while_end_2:
  lw ra, 60(sp)
  addi sp, sp, 64
  ret
  .text
  .globl main
main:
main_entry:
  li a0, 33
  ret
