  .text
  .globl init
init:
  addi sp, sp, -144
init_entry:
  mv t0, a0
  sw t0, 0(sp)
  li t0, 0
  sw t0, 4(sp)
  j init_while_entry_0
init_while_entry_0:
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
  bnez t0, init_while_body_1
  j init_while_end_2
init_while_body_1:
  li t0, 0
  sw t0, 20(sp)
  j init_while_entry_3
init_while_end_2:
  addi sp, sp, 144
  ret
init_while_entry_3:
  lw t0, 20(sp)
  sw t0, 24(sp)
  lw t0, 24(sp)
  li t1, 10
  slt t0, t0, t1
  sw t0, 28(sp)
  lw t0, 28(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 32(sp)
  lw t0, 32(sp)
  bnez t0, init_while_body_4
  j init_while_end_5
init_while_body_4:
  li t0, 0
  sw t0, 36(sp)
  j init_while_entry_6
init_while_end_5:
  lw t0, 4(sp)
  sw t0, 40(sp)
  lw t0, 40(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 44(sp)
  lw t0, 44(sp)
  sw t0, 4(sp)
  j init_while_entry_0
init_while_entry_6:
  lw t0, 36(sp)
  sw t0, 48(sp)
  lw t0, 48(sp)
  li t1, 10
  slt t0, t0, t1
  sw t0, 52(sp)
  lw t0, 52(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 56(sp)
  lw t0, 56(sp)
  bnez t0, init_while_body_7
  j init_while_end_8
init_while_body_7:
  lw t0, 4(sp)
  sw t0, 60(sp)
  lw t0, 60(sp)
  li t1, 100
  mul t0, t0, t1
  sw t0, 64(sp)
  lw t0, 20(sp)
  sw t0, 68(sp)
  lw t0, 68(sp)
  li t1, 10
  mul t0, t0, t1
  sw t0, 72(sp)
  lw t0, 64(sp)
  lw t1, 72(sp)
  add t0, t0, t1
  sw t0, 76(sp)
  lw t0, 36(sp)
  sw t0, 80(sp)
  lw t0, 76(sp)
  lw t1, 80(sp)
  add t0, t0, t1
  sw t0, 84(sp)
  flw ft0, 0(sp)
  fsw ft0, 88(sp)
  lw t0, 4(sp)
  sw t0, 92(sp)
  lw t0, 88(sp)
  lw t1, 92(sp)
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 96(sp)
  lw t0, 20(sp)
  sw t0, 100(sp)
  lw t0, 96(sp)
  lw t1, 100(sp)
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 104(sp)
  lw t0, 36(sp)
  sw t0, 108(sp)
  lw t0, 104(sp)
  lw t1, 108(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 112(sp)
  lw t0, 84(sp)
  fcvt.s.w ft0, t0
  lw t1, 112(sp)
  fsw ft0, 0(t1)
  lw t0, 36(sp)
  sw t0, 116(sp)
  lw t0, 116(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 120(sp)
  lw t0, 120(sp)
  sw t0, 36(sp)
  j init_while_entry_6
init_while_end_8:
  lw t0, 20(sp)
  sw t0, 124(sp)
  lw t0, 124(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 128(sp)
  lw t0, 128(sp)
  sw t0, 20(sp)
  j init_while_entry_3
  .text
  .globl f1
f1:
  addi sp, sp, -208
f1_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  mv t0, a2
  sw t0, 8(sp)
  mv t0, a3
  sw t0, 12(sp)
  mv t0, a4
  sw t0, 16(sp)
  mv t0, a5
  sw t0, 20(sp)
  mv t0, a6
  sw t0, 24(sp)
  mv t0, a7
  sw t0, 28(sp)
  lw t0, 208(sp)
  sw t0, 32(sp)
  lw t0, 212(sp)
  sw t0, 36(sp)
  flw ft0, 0(sp)
  fsw ft0, 40(sp)
  lw t0, 40(sp)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 44(sp)
  lw t1, 44(sp)
  flw ft0, 0(t1)
  fsw ft0, 48(sp)
  flw ft0, 4(sp)
  fsw ft0, 52(sp)
  lw t0, 52(sp)
  li t1, 1
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 56(sp)
  lw t1, 56(sp)
  flw ft0, 0(t1)
  fsw ft0, 60(sp)
  flw ft0, 48(sp)
  flw ft1, 60(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 64(sp)
  flw ft0, 8(sp)
  fsw ft0, 68(sp)
  lw t0, 68(sp)
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 72(sp)
  lw t1, 72(sp)
  flw ft0, 0(t1)
  fsw ft0, 76(sp)
  flw ft0, 64(sp)
  flw ft1, 76(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 80(sp)
  flw ft0, 12(sp)
  fsw ft0, 84(sp)
  lw t0, 84(sp)
  li t1, 3
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 88(sp)
  lw t1, 88(sp)
  flw ft0, 0(t1)
  fsw ft0, 92(sp)
  flw ft0, 80(sp)
  flw ft1, 92(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 96(sp)
  flw ft0, 16(sp)
  fsw ft0, 100(sp)
  lw t0, 100(sp)
  li t1, 4
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 104(sp)
  lw t1, 104(sp)
  flw ft0, 0(t1)
  fsw ft0, 108(sp)
  flw ft0, 96(sp)
  flw ft1, 108(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 112(sp)
  flw ft0, 20(sp)
  fsw ft0, 116(sp)
  lw t0, 116(sp)
  li t1, 5
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 120(sp)
  lw t1, 120(sp)
  flw ft0, 0(t1)
  fsw ft0, 124(sp)
  flw ft0, 112(sp)
  flw ft1, 124(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 128(sp)
  flw ft0, 24(sp)
  fsw ft0, 132(sp)
  lw t0, 132(sp)
  li t1, 6
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 136(sp)
  lw t1, 136(sp)
  flw ft0, 0(t1)
  fsw ft0, 140(sp)
  flw ft0, 128(sp)
  flw ft1, 140(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 144(sp)
  flw ft0, 28(sp)
  fsw ft0, 148(sp)
  lw t0, 148(sp)
  li t1, 7
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 152(sp)
  lw t1, 152(sp)
  flw ft0, 0(t1)
  fsw ft0, 156(sp)
  flw ft0, 144(sp)
  flw ft1, 156(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 160(sp)
  flw ft0, 32(sp)
  fsw ft0, 164(sp)
  lw t0, 164(sp)
  li t1, 8
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 168(sp)
  lw t1, 168(sp)
  flw ft0, 0(t1)
  fsw ft0, 172(sp)
  flw ft0, 160(sp)
  flw ft1, 172(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 176(sp)
  flw ft0, 36(sp)
  fsw ft0, 180(sp)
  lw t0, 180(sp)
  li t1, 9
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 184(sp)
  lw t1, 184(sp)
  flw ft0, 0(t1)
  fsw ft0, 188(sp)
  flw ft0, 176(sp)
  flw ft1, 188(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 192(sp)
  flw ft0, 192(sp)
  fcvt.w.s a0, ft0, rtz
  addi sp, sp, 208
  ret
  .text
  .globl f2
f2:
  addi sp, sp, -208
f2_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  mv t0, a2
  fmv.w.x ft0, t0
  fsw ft0, 8(sp)
  mv t0, a3
  sw t0, 12(sp)
  mv t0, a4
  sw t0, 16(sp)
  mv t0, a5
  sw t0, 20(sp)
  mv t0, a6
  sw t0, 24(sp)
  mv t0, a7
  sw t0, 28(sp)
  lw t0, 208(sp)
  fmv.w.x ft0, t0
  fsw ft0, 32(sp)
  lw t0, 212(sp)
  sw t0, 36(sp)
  flw ft0, 0(sp)
  fsw ft0, 40(sp)
  lw t0, 40(sp)
  li t1, 0
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 44(sp)
  lw t0, 44(sp)
  li t1, 9
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 48(sp)
  lw t1, 48(sp)
  flw ft0, 0(t1)
  fsw ft0, 52(sp)
  flw ft0, 4(sp)
  fsw ft0, 56(sp)
  lw t0, 56(sp)
  li t1, 1
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 60(sp)
  lw t1, 60(sp)
  flw ft0, 0(t1)
  fsw ft0, 64(sp)
  flw ft0, 52(sp)
  flw ft1, 64(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 68(sp)
  flw ft0, 8(sp)
  fsw ft0, 72(sp)
  flw ft0, 68(sp)
  flw ft1, 72(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 76(sp)
  flw ft0, 12(sp)
  fsw ft0, 80(sp)
  lw t0, 80(sp)
  li t1, 3
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 84(sp)
  lw t1, 84(sp)
  flw ft0, 0(t1)
  fsw ft0, 88(sp)
  flw ft0, 76(sp)
  flw ft1, 88(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 92(sp)
  flw ft0, 16(sp)
  fsw ft0, 96(sp)
  lw t0, 96(sp)
  li t1, 4
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 100(sp)
  lw t1, 100(sp)
  flw ft0, 0(t1)
  fsw ft0, 104(sp)
  flw ft0, 92(sp)
  flw ft1, 104(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 108(sp)
  flw ft0, 20(sp)
  fsw ft0, 112(sp)
  lw t0, 112(sp)
  li t1, 5
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 116(sp)
  lw t0, 116(sp)
  li t1, 5
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 120(sp)
  lw t0, 120(sp)
  li t1, 5
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 124(sp)
  lw t1, 124(sp)
  flw ft0, 0(t1)
  fsw ft0, 128(sp)
  flw ft0, 108(sp)
  flw ft1, 128(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 132(sp)
  flw ft0, 24(sp)
  fsw ft0, 136(sp)
  lw t0, 136(sp)
  li t1, 6
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 140(sp)
  lw t1, 140(sp)
  flw ft0, 0(t1)
  fsw ft0, 144(sp)
  flw ft0, 132(sp)
  flw ft1, 144(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 148(sp)
  flw ft0, 28(sp)
  fsw ft0, 152(sp)
  lw t0, 152(sp)
  li t1, 7
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 156(sp)
  lw t1, 156(sp)
  flw ft0, 0(t1)
  fsw ft0, 160(sp)
  flw ft0, 148(sp)
  flw ft1, 160(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 164(sp)
  flw ft0, 32(sp)
  fsw ft0, 168(sp)
  flw ft0, 164(sp)
  flw ft1, 168(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 172(sp)
  flw ft0, 36(sp)
  fsw ft0, 176(sp)
  lw t0, 176(sp)
  li t1, 9
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 180(sp)
  lw t0, 180(sp)
  li t1, 8
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 184(sp)
  lw t1, 184(sp)
  flw ft0, 0(t1)
  fsw ft0, 188(sp)
  flw ft0, 172(sp)
  flw ft1, 188(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 192(sp)
  flw ft0, 192(sp)
  fcvt.w.s a0, ft0, rtz
  addi sp, sp, 208
  ret
  .text
  .globl main
main:
  li t0, -4288
  add sp, sp, t0
  li t2, 4284
  add t2, sp, t2
  sw ra, 0(t2)
main_entry:
  li t0, 0
  fcvt.s.w ft0, t0
  li t2, 4008
  add t2, sp, t2
  fsw ft0, 0(t2)
  addi t0, sp, 8
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4012
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4012
  add t2, sp, t2
  lw a0, 0(t2)
  call init
  li t2, 4008
  add t2, sp, t2
  flw ft0, 0(t2)
  li t2, 4016
  add t2, sp, t2
  fsw ft0, 0(t2)
  addi t0, sp, 8
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4020
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4020
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4024
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4024
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4028
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 1
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4032
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4032
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 1
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4036
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4036
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4040
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 2
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4044
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4044
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 2
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4048
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4048
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4052
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 3
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4056
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4056
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 3
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4060
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4060
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4064
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 4
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4068
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4068
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 4
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4072
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4072
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4076
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 5
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4080
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4080
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 5
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4084
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4084
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4088
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 6
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4092
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4092
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 6
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4096
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4096
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4100
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 7
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4104
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4104
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 7
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4108
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4108
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4112
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 8
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4116
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4116
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 8
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4120
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4120
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4124
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 9
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4128
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4128
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 9
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4132
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4132
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4136
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4028
  add t2, sp, t2
  lw a0, 0(t2)
  li t2, 4040
  add t2, sp, t2
  lw a1, 0(t2)
  li t2, 4052
  add t2, sp, t2
  lw a2, 0(t2)
  li t2, 4064
  add t2, sp, t2
  lw a3, 0(t2)
  li t2, 4076
  add t2, sp, t2
  lw a4, 0(t2)
  li t2, 4088
  add t2, sp, t2
  lw a5, 0(t2)
  li t2, 4100
  add t2, sp, t2
  lw a6, 0(t2)
  li t2, 4112
  add t2, sp, t2
  lw a7, 0(t2)
  li t2, 4124
  add t2, sp, t2
  lw t0, 0(t2)
  sw t0, 0(sp)
  li t2, 4136
  add t2, sp, t2
  lw t0, 0(t2)
  sw t0, 4(sp)
  call f1
  li t2, 4140
  add t2, sp, t2
  sw a0, 0(t2)
  li t2, 4016
  add t2, sp, t2
  flw ft0, 0(t2)
  li t2, 4140
  add t2, sp, t2
  lw t0, 0(t2)
  fcvt.s.w ft1, t0
  fadd.s ft0, ft0, ft1
  li t2, 4144
  add t2, sp, t2
  fsw ft0, 0(t2)
  li t2, 4144
  add t2, sp, t2
  flw ft0, 0(t2)
  li t2, 4008
  add t2, sp, t2
  fsw ft0, 0(t2)
  li t2, 4008
  add t2, sp, t2
  flw ft0, 0(t2)
  li t2, 4148
  add t2, sp, t2
  fsw ft0, 0(t2)
  addi t0, sp, 8
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4152
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4152
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4156
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 1
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4160
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4160
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 1
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4164
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4164
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4168
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 2
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4172
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4172
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 2
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4176
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4176
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4180
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4180
  add t2, sp, t2
  lw t1, 0(t2)
  flw ft0, 0(t1)
  li t2, 4184
  add t2, sp, t2
  fsw ft0, 0(t2)
  addi t0, sp, 8
  li t1, 3
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4188
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4188
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 3
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4192
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4192
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4196
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 4
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4200
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4200
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 4
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4204
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4204
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4208
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4212
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 6
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4216
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4216
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 6
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4220
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4220
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4224
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 7
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4228
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4228
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 7
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4232
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4232
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4236
  add t2, sp, t2
  sw t0, 0(t2)
  addi t0, sp, 8
  li t1, 8
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4240
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4240
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 8
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4244
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4244
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 8
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4248
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4248
  add t2, sp, t2
  lw t1, 0(t2)
  flw ft0, 0(t1)
  li t2, 4252
  add t2, sp, t2
  fsw ft0, 0(t2)
  addi t0, sp, 8
  li t1, 9
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4256
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4256
  add t2, sp, t2
  lw t0, 0(t2)
  li t1, 0
  li t2, 40
  mul t1, t1, t2
  add t0, t0, t1
  li t2, 4260
  add t2, sp, t2
  sw t0, 0(t2)
  li t2, 4156
  add t2, sp, t2
  lw a0, 0(t2)
  li t2, 4168
  add t2, sp, t2
  lw a1, 0(t2)
  li t2, 4184
  add t2, sp, t2
  lw a2, 0(t2)
  li t2, 4196
  add t2, sp, t2
  lw a3, 0(t2)
  li t2, 4208
  add t2, sp, t2
  lw a4, 0(t2)
  li t2, 4212
  add t2, sp, t2
  lw a5, 0(t2)
  li t2, 4224
  add t2, sp, t2
  lw a6, 0(t2)
  li t2, 4236
  add t2, sp, t2
  lw a7, 0(t2)
  li t2, 4252
  add t2, sp, t2
  lw t0, 0(t2)
  sw t0, 0(sp)
  li t2, 4260
  add t2, sp, t2
  lw t0, 0(t2)
  sw t0, 4(sp)
  call f2
  li t2, 4264
  add t2, sp, t2
  sw a0, 0(t2)
  li t2, 4148
  add t2, sp, t2
  flw ft0, 0(t2)
  li t2, 4264
  add t2, sp, t2
  lw t0, 0(t2)
  fcvt.s.w ft1, t0
  fadd.s ft0, ft0, ft1
  li t2, 4268
  add t2, sp, t2
  fsw ft0, 0(t2)
  li t2, 4268
  add t2, sp, t2
  flw ft0, 0(t2)
  li t2, 4008
  add t2, sp, t2
  fsw ft0, 0(t2)
  li t2, 4008
  add t2, sp, t2
  flw ft0, 0(t2)
  li t2, 4272
  add t2, sp, t2
  fsw ft0, 0(t2)
  li t2, 4272
  add t2, sp, t2
  flw fa0, 0(t2)
  call putfloat
  li a0, 10
  call putch
  li a0, 0
  li t2, 4284
  add t2, sp, t2
  lw ra, 0(t2)
  li t0, 4288
  add sp, sp, t0
  ret
