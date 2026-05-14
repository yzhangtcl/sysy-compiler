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
  li t0, 1093937398
  fmv.w.x ft0, t0
  fsw ft0, 16(sp)
  flw ft0, 16(sp)
  fsw ft0, 0(sp)
  li t0, 120
  sw t0, 20(sp)
  flw ft0, 0(sp)
  fsw ft0, 28(sp)
  flw ft0, 8(sp)
  fsw ft0, 32(sp)
  flw ft0, 28(sp)
  flw ft1, 32(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 36(sp)
  lw t0, 20(sp)
  sw t0, 40(sp)
  flw ft0, 36(sp)
  lw t0, 40(sp)
  fcvt.s.w ft1, t0
  fmul.s ft0, ft0, ft1
  fsw ft0, 44(sp)
  li t0, 1070385542
  fmv.w.x ft0, t0
  fsw ft0, 48(sp)
  flw ft0, 44(sp)
  flw ft1, 48(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 52(sp)
  flw ft0, 52(sp)
  li t0, 10
  fcvt.s.w ft1, t0
  fadd.s ft0, ft0, ft1
  fsw ft0, 56(sp)
  flw ft0, 56(sp)
  fcvt.w.s t0, ft0, rtz
  sw t0, 24(sp)
  lw t0, 24(sp)
  sw t0, 60(sp)
  lw a0, 60(sp)
  addi sp, sp, 64
  ret
