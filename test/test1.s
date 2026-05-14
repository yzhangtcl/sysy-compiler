  .text
  .globl main
main:
  addi sp, sp, -64
  li t0, 1067534254
  fmv.w.x ft0, t0
  fsw ft0, 4(sp)
  flw ft0, 4(sp)
  fsw ft0, 0(sp)
  li t0, 1074161254
  fmv.w.x ft0, t0
  fsw ft0, 12(sp)
  flw ft0, 12(sp)
  fsw ft0, 8(sp)
  li t0, 120
  sw t0, 16(sp)
  flw ft0, 0(sp)
  fsw ft0, 24(sp)
  flw ft0, 8(sp)
  fsw ft0, 28(sp)
  flw ft0, 24(sp)
  flw ft1, 28(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 32(sp)
  lw t0, 16(sp)
  sw t0, 36(sp)
  flw ft0, 32(sp)
  lw t0, 36(sp)
  fcvt.s.w ft1, t0
  fmul.s ft0, ft0, ft1
  fsw ft0, 40(sp)
  li t0, 1070385542
  fmv.w.x ft0, t0
  fsw ft0, 44(sp)
  flw ft0, 40(sp)
  flw ft1, 44(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 48(sp)
  flw ft0, 48(sp)
  li t0, 120
  fcvt.s.w ft1, t0
  fadd.s ft0, ft0, ft1
  fsw ft0, 52(sp)
  flw ft0, 52(sp)
  fcvt.w.s t0, ft0, rtz
  sw t0, 20(sp)
  lw t0, 20(sp)
  sw t0, 56(sp)
  lw a0, 56(sp)
  addi sp, sp, 64
  ret
