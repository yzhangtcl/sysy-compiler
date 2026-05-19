  .data
  .globl a
a:
  .zero 1600
  .text
  .globl localEnergy
localEnergy:
  addi sp, sp, -288
  sw ra, 284(sp)
localEnergy_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  mv t0, a2
  sw t0, 8(sp)
  lw t0, 0(sp)
  sw t0, 28(sp)
  la t0, a
  lw t1, 28(sp)
  li t2, 80
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 32(sp)
  lw t0, 4(sp)
  sw t0, 36(sp)
  lw t0, 32(sp)
  lw t1, 36(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 40(sp)
  lw t1, 40(sp)
  lw t0, 0(t1)
  sw t0, 44(sp)
  lw t0, 44(sp)
  sw t0, 12(sp)
  lw t0, 8(sp)
  sw t0, 48(sp)
  lw t0, 48(sp)
  li t1, 0
  sgt t0, t0, t1
  seqz t0, t0
  sw t0, 52(sp)
  lw t0, 52(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 56(sp)
  lw t0, 56(sp)
  bnez t0, localEnergy_if_then_0
  j localEnergy_if_end_1
localEnergy_if_then_0:
  lw t0, 12(sp)
  sw t0, 60(sp)
  lw a0, 60(sp)
  lw ra, 284(sp)
  addi sp, sp, 288
  ret
localEnergy_if_end_1:
  lw t0, 12(sp)
  sw t0, 64(sp)
  lw t0, 64(sp)
  sw t0, 16(sp)
  lw t0, 0(sp)
  sw t0, 68(sp)
  lw t0, 68(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 72(sp)
  lw t0, 72(sp)
  sw t0, 20(sp)
  lw t0, 4(sp)
  sw t0, 76(sp)
  lw t0, 76(sp)
  sw t0, 24(sp)
  lw t0, 20(sp)
  sw t0, 80(sp)
  lw t0, 80(sp)
  li t1, 0
  slt t0, t0, t1
  seqz t0, t0
  sw t0, 84(sp)
  lw t0, 84(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 88(sp)
  lw t0, 88(sp)
  bnez t0, localEnergy_if_then_2
  j localEnergy_if_end_3
localEnergy_if_then_2:
  lw t0, 16(sp)
  sw t0, 92(sp)
  lw t0, 20(sp)
  sw t0, 96(sp)
  lw t0, 24(sp)
  sw t0, 100(sp)
  lw t0, 8(sp)
  sw t0, 104(sp)
  lw t0, 104(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 108(sp)
  lw a0, 96(sp)
  lw a1, 100(sp)
  lw a2, 108(sp)
  call localEnergy
  sw a0, 112(sp)
  lw t0, 92(sp)
  lw t1, 112(sp)
  add t0, t0, t1
  sw t0, 116(sp)
  lw t0, 116(sp)
  sw t0, 16(sp)
  j localEnergy_if_end_3
localEnergy_if_end_3:
  lw t0, 0(sp)
  sw t0, 120(sp)
  lw t0, 120(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 124(sp)
  lw t0, 124(sp)
  sw t0, 20(sp)
  lw t0, 20(sp)
  sw t0, 128(sp)
  lw t0, 128(sp)
  li t1, 20
  slt t0, t0, t1
  sw t0, 132(sp)
  lw t0, 132(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 136(sp)
  lw t0, 136(sp)
  bnez t0, localEnergy_if_then_4
  j localEnergy_if_end_5
localEnergy_if_then_4:
  lw t0, 16(sp)
  sw t0, 140(sp)
  lw t0, 20(sp)
  sw t0, 144(sp)
  lw t0, 24(sp)
  sw t0, 148(sp)
  lw t0, 8(sp)
  sw t0, 152(sp)
  lw t0, 152(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 156(sp)
  lw a0, 144(sp)
  lw a1, 148(sp)
  lw a2, 156(sp)
  call localEnergy
  sw a0, 160(sp)
  lw t0, 140(sp)
  lw t1, 160(sp)
  add t0, t0, t1
  sw t0, 164(sp)
  lw t0, 164(sp)
  sw t0, 16(sp)
  j localEnergy_if_end_5
localEnergy_if_end_5:
  lw t0, 0(sp)
  sw t0, 168(sp)
  lw t0, 168(sp)
  sw t0, 20(sp)
  lw t0, 4(sp)
  sw t0, 172(sp)
  lw t0, 172(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 176(sp)
  lw t0, 176(sp)
  sw t0, 24(sp)
  lw t0, 24(sp)
  sw t0, 180(sp)
  lw t0, 180(sp)
  li t1, 0
  slt t0, t0, t1
  seqz t0, t0
  sw t0, 184(sp)
  lw t0, 184(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 188(sp)
  lw t0, 188(sp)
  bnez t0, localEnergy_if_then_6
  j localEnergy_if_end_7
localEnergy_if_then_6:
  lw t0, 16(sp)
  sw t0, 192(sp)
  lw t0, 20(sp)
  sw t0, 196(sp)
  lw t0, 24(sp)
  sw t0, 200(sp)
  lw t0, 8(sp)
  sw t0, 204(sp)
  lw t0, 204(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 208(sp)
  lw a0, 196(sp)
  lw a1, 200(sp)
  lw a2, 208(sp)
  call localEnergy
  sw a0, 212(sp)
  lw t0, 192(sp)
  lw t1, 212(sp)
  add t0, t0, t1
  sw t0, 216(sp)
  lw t0, 216(sp)
  sw t0, 16(sp)
  j localEnergy_if_end_7
localEnergy_if_end_7:
  lw t0, 4(sp)
  sw t0, 220(sp)
  lw t0, 220(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 224(sp)
  lw t0, 224(sp)
  sw t0, 24(sp)
  lw t0, 24(sp)
  sw t0, 228(sp)
  lw t0, 228(sp)
  li t1, 20
  slt t0, t0, t1
  sw t0, 232(sp)
  lw t0, 232(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 236(sp)
  lw t0, 236(sp)
  bnez t0, localEnergy_if_then_8
  j localEnergy_if_end_9
localEnergy_if_then_8:
  lw t0, 16(sp)
  sw t0, 240(sp)
  lw t0, 20(sp)
  sw t0, 244(sp)
  lw t0, 24(sp)
  sw t0, 248(sp)
  lw t0, 8(sp)
  sw t0, 252(sp)
  lw t0, 252(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 256(sp)
  lw a0, 244(sp)
  lw a1, 248(sp)
  lw a2, 256(sp)
  call localEnergy
  sw a0, 260(sp)
  lw t0, 240(sp)
  lw t1, 260(sp)
  add t0, t0, t1
  sw t0, 264(sp)
  lw t0, 264(sp)
  sw t0, 16(sp)
  j localEnergy_if_end_9
localEnergy_if_end_9:
  lw t0, 16(sp)
  sw t0, 268(sp)
  lw a0, 268(sp)
  lw ra, 284(sp)
  addi sp, sp, 288
  ret
  .text
  .globl main
main:
  addi sp, sp, -80
main_entry:
  li t0, 182
  li t1, 8333
  add t0, t0, t1
  sw t0, 4(sp)
  lw t0, 4(sp)
  sw t0, 0(sp)
  li t0, 1067534254
  fmv.w.x ft0, t0
  fsw ft0, 12(sp)
  flw ft0, 12(sp)
  fsw ft0, 8(sp)
  li t0, 1074161254
  fmv.w.x ft0, t0
  fsw ft0, 20(sp)
  flw ft0, 20(sp)
  fsw ft0, 16(sp)
  li t0, 1093937398
  fmv.w.x ft0, t0
  fsw ft0, 24(sp)
  flw ft0, 24(sp)
  fsw ft0, 8(sp)
  li t0, 120
  sw t0, 28(sp)
  flw ft0, 8(sp)
  fsw ft0, 36(sp)
  flw ft0, 16(sp)
  fsw ft0, 40(sp)
  flw ft0, 36(sp)
  flw ft1, 40(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 44(sp)
  lw t0, 28(sp)
  sw t0, 48(sp)
  flw ft0, 44(sp)
  lw t0, 48(sp)
  fcvt.s.w ft1, t0
  fmul.s ft0, ft0, ft1
  fsw ft0, 52(sp)
  li t0, 1070385542
  fmv.w.x ft0, t0
  fsw ft0, 56(sp)
  flw ft0, 52(sp)
  flw ft1, 56(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 60(sp)
  flw ft0, 60(sp)
  li t0, 10
  fcvt.s.w ft1, t0
  fadd.s ft0, ft0, ft1
  fsw ft0, 64(sp)
  flw ft0, 64(sp)
  fcvt.w.s t0, ft0, rtz
  sw t0, 32(sp)
  lw t0, 32(sp)
  sw t0, 68(sp)
  lw a0, 68(sp)
  addi sp, sp, 80
  ret
