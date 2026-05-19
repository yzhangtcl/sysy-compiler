  .text
  .globl main
main:
  addi sp, sp, -128
main_entry:
  addi t0, sp, 0
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 40(sp)
  li t0, 1067534254
  fmv.w.x ft0, t0
  fsw ft0, 44(sp)
  flw ft0, 44(sp)
  lw t1, 40(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 1
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 48(sp)
  li t0, 1074161254
  fmv.w.x ft0, t0
  fsw ft0, 52(sp)
  flw ft0, 52(sp)
  lw t1, 48(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 56(sp)
  li t0, 1080033280
  fmv.w.x ft0, t0
  fsw ft0, 60(sp)
  flw ft0, 60(sp)
  lw t1, 56(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 3
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 64(sp)
  li t0, 1083388723
  fmv.w.x ft0, t0
  fsw ft0, 68(sp)
  flw ft0, 68(sp)
  lw t1, 64(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 4
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 72(sp)
  li t0, 1085695590
  fmv.w.x ft0, t0
  fsw ft0, 76(sp)
  flw ft0, 76(sp)
  lw t1, 72(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 5
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 80(sp)
  li t0, 1088002458
  fmv.w.x ft0, t0
  fsw ft0, 84(sp)
  flw ft0, 84(sp)
  lw t1, 80(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 6
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 88(sp)
  li t0, 1090309325
  fmv.w.x ft0, t0
  fsw ft0, 92(sp)
  flw ft0, 92(sp)
  lw t1, 88(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 7
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 96(sp)
  li t0, 1090519040
  fmv.w.x ft0, t0
  fsw ft0, 100(sp)
  flw ft0, 100(sp)
  lw t1, 96(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 8
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 104(sp)
  li t0, 1091672474
  fmv.w.x ft0, t0
  fsw ft0, 108(sp)
  flw ft0, 108(sp)
  lw t1, 104(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 9
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 112(sp)
  li t0, 1092825907
  fmv.w.x ft0, t0
  fsw ft0, 116(sp)
  flw ft0, 116(sp)
  lw t1, 112(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 5
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 120(sp)
  lw t1, 120(sp)
  flw ft0, 0(t1)
  fsw ft0, 124(sp)
  flw ft0, 124(sp)
  fcvt.w.s a0, ft0, rtz
  addi sp, sp, 128
  ret
