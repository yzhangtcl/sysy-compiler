  .data
  .globl n
n:
  .zero 4
  .text
  .globl swap
swap:
  addi sp, sp, -80
swap_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  mv t0, a2
  sw t0, 8(sp)
  flw ft0, 0(sp)
  fsw ft0, 16(sp)
  lw t0, 4(sp)
  sw t0, 20(sp)
  lw t0, 16(sp)
  lw t1, 20(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 24(sp)
  lw t1, 24(sp)
  flw ft0, 0(t1)
  fsw ft0, 28(sp)
  flw ft0, 28(sp)
  fsw ft0, 12(sp)
  flw ft0, 0(sp)
  fsw ft0, 32(sp)
  lw t0, 8(sp)
  sw t0, 36(sp)
  lw t0, 32(sp)
  lw t1, 36(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 40(sp)
  lw t1, 40(sp)
  flw ft0, 0(t1)
  fsw ft0, 44(sp)
  flw ft0, 0(sp)
  fsw ft0, 48(sp)
  lw t0, 4(sp)
  sw t0, 52(sp)
  lw t0, 48(sp)
  lw t1, 52(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 56(sp)
  flw ft0, 44(sp)
  lw t1, 56(sp)
  fsw ft0, 0(t1)
  flw ft0, 12(sp)
  fsw ft0, 60(sp)
  flw ft0, 0(sp)
  fsw ft0, 64(sp)
  lw t0, 8(sp)
  sw t0, 68(sp)
  lw t0, 64(sp)
  lw t1, 68(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 72(sp)
  flw ft0, 60(sp)
  lw t1, 72(sp)
  fsw ft0, 0(t1)
  li a0, 0
  addi sp, sp, 80
  ret
  .text
  .globl heap_ajust
heap_ajust:
  addi sp, sp, -224
  sw ra, 220(sp)
heap_ajust_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  mv t0, a2
  sw t0, 8(sp)
  lw t0, 4(sp)
  sw t0, 16(sp)
  lw t0, 16(sp)
  sw t0, 12(sp)
  lw t0, 12(sp)
  sw t0, 24(sp)
  lw t0, 24(sp)
  li t1, 2
  mul t0, t0, t1
  sw t0, 28(sp)
  lw t0, 28(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 32(sp)
  lw t0, 32(sp)
  sw t0, 20(sp)
  j heap_ajust_while_entry_0
heap_ajust_while_entry_0:
  lw t0, 20(sp)
  sw t0, 36(sp)
  lw t0, 8(sp)
  sw t0, 40(sp)
  lw t0, 40(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 44(sp)
  lw t0, 36(sp)
  lw t1, 44(sp)
  slt t0, t0, t1
  sw t0, 48(sp)
  lw t0, 48(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 52(sp)
  lw t0, 52(sp)
  bnez t0, heap_ajust_while_body_1
  j heap_ajust_while_end_2
heap_ajust_while_body_1:
  lw t0, 20(sp)
  sw t0, 56(sp)
  lw t0, 8(sp)
  sw t0, 60(sp)
  lw t0, 56(sp)
  lw t1, 60(sp)
  slt t0, t0, t1
  sw t0, 64(sp)
  lw t0, 64(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 68(sp)
  lw t0, 68(sp)
  bnez t0, heap_ajust_sc_and_rhs_4
  j heap_ajust_sc_and_false_5
heap_ajust_while_end_2:
  li a0, 0
  lw ra, 220(sp)
  addi sp, sp, 224
  ret
heap_ajust_sc_and_rhs_4:
  flw ft0, 0(sp)
  fsw ft0, 76(sp)
  lw t0, 20(sp)
  sw t0, 80(sp)
  lw t0, 76(sp)
  lw t1, 80(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 84(sp)
  lw t1, 84(sp)
  flw ft0, 0(t1)
  fsw ft0, 88(sp)
  flw ft0, 0(sp)
  fsw ft0, 92(sp)
  lw t0, 20(sp)
  sw t0, 96(sp)
  lw t0, 96(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 100(sp)
  lw t0, 92(sp)
  lw t1, 100(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 104(sp)
  lw t1, 104(sp)
  flw ft0, 0(t1)
  fsw ft0, 108(sp)
  flw ft0, 88(sp)
  flw ft1, 108(sp)
  flt.s t0, ft0, ft1
  sw t0, 112(sp)
  lw t0, 112(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 116(sp)
  lw t0, 116(sp)
  sw t0, 72(sp)
  j heap_ajust_sc_end_3
heap_ajust_sc_and_false_5:
  li t0, 0
  sw t0, 72(sp)
  j heap_ajust_sc_end_3
heap_ajust_sc_end_3:
  lw t0, 72(sp)
  sw t0, 120(sp)
  lw t0, 120(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 124(sp)
  lw t0, 124(sp)
  bnez t0, heap_ajust_if_then_6
  j heap_ajust_if_end_7
heap_ajust_if_then_6:
  lw t0, 20(sp)
  sw t0, 128(sp)
  lw t0, 128(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 132(sp)
  lw t0, 132(sp)
  sw t0, 20(sp)
  j heap_ajust_if_end_7
heap_ajust_if_end_7:
  flw ft0, 0(sp)
  fsw ft0, 136(sp)
  lw t0, 12(sp)
  sw t0, 140(sp)
  lw t0, 136(sp)
  lw t1, 140(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 144(sp)
  lw t1, 144(sp)
  flw ft0, 0(t1)
  fsw ft0, 148(sp)
  flw ft0, 0(sp)
  fsw ft0, 152(sp)
  lw t0, 20(sp)
  sw t0, 156(sp)
  lw t0, 152(sp)
  lw t1, 156(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 160(sp)
  lw t1, 160(sp)
  flw ft0, 0(t1)
  fsw ft0, 164(sp)
  flw ft0, 148(sp)
  flw ft1, 164(sp)
  flt.s t0, ft1, ft0
  sw t0, 168(sp)
  lw t0, 168(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 172(sp)
  lw t0, 172(sp)
  bnez t0, heap_ajust_if_then_8
  j heap_ajust_if_else_10
heap_ajust_if_then_8:
  li a0, 0
  lw ra, 220(sp)
  addi sp, sp, 224
  ret
heap_ajust_if_else_10:
  flw ft0, 0(sp)
  fsw ft0, 176(sp)
  lw t0, 12(sp)
  sw t0, 180(sp)
  lw t0, 20(sp)
  sw t0, 184(sp)
  lw a0, 176(sp)
  lw a1, 180(sp)
  lw a2, 184(sp)
  call swap
  sw a0, 188(sp)
  lw t0, 188(sp)
  sw t0, 12(sp)
  lw t0, 20(sp)
  sw t0, 192(sp)
  lw t0, 192(sp)
  sw t0, 12(sp)
  lw t0, 12(sp)
  sw t0, 196(sp)
  lw t0, 196(sp)
  li t1, 2
  mul t0, t0, t1
  sw t0, 200(sp)
  lw t0, 200(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 204(sp)
  lw t0, 204(sp)
  sw t0, 20(sp)
  j heap_ajust_if_end_9
heap_ajust_if_end_9:
  j heap_ajust_while_entry_0
  .text
  .globl heap_sort
heap_sort:
  addi sp, sp, -160
  sw ra, 156(sp)
heap_sort_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  lw t0, 4(sp)
  sw t0, 16(sp)
  lw t0, 16(sp)
  li t1, 2
  div t0, t0, t1
  sw t0, 20(sp)
  lw t0, 20(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 24(sp)
  lw t0, 24(sp)
  sw t0, 8(sp)
  j heap_sort_while_entry_0
heap_sort_while_entry_0:
  lw t0, 8(sp)
  sw t0, 28(sp)
  li t0, 0
  li t1, 1
  sub t0, t0, t1
  sw t0, 32(sp)
  lw t0, 28(sp)
  lw t1, 32(sp)
  sgt t0, t0, t1
  sw t0, 36(sp)
  lw t0, 36(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 40(sp)
  lw t0, 40(sp)
  bnez t0, heap_sort_while_body_1
  j heap_sort_while_end_2
heap_sort_while_body_1:
  lw t0, 4(sp)
  sw t0, 44(sp)
  lw t0, 44(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 48(sp)
  lw t0, 48(sp)
  sw t0, 12(sp)
  flw ft0, 0(sp)
  fsw ft0, 52(sp)
  lw t0, 8(sp)
  sw t0, 56(sp)
  lw t0, 12(sp)
  sw t0, 60(sp)
  lw a0, 52(sp)
  lw a1, 56(sp)
  lw a2, 60(sp)
  call heap_ajust
  sw a0, 64(sp)
  lw t0, 64(sp)
  sw t0, 12(sp)
  lw t0, 8(sp)
  sw t0, 68(sp)
  lw t0, 68(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 72(sp)
  lw t0, 72(sp)
  sw t0, 8(sp)
  j heap_sort_while_entry_0
heap_sort_while_end_2:
  lw t0, 4(sp)
  sw t0, 76(sp)
  lw t0, 76(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 80(sp)
  lw t0, 80(sp)
  sw t0, 8(sp)
  j heap_sort_while_entry_3
heap_sort_while_entry_3:
  lw t0, 8(sp)
  sw t0, 84(sp)
  lw t0, 84(sp)
  li t1, 0
  sgt t0, t0, t1
  sw t0, 88(sp)
  lw t0, 88(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 92(sp)
  lw t0, 92(sp)
  bnez t0, heap_sort_while_body_4
  j heap_sort_while_end_5
heap_sort_while_body_4:
  li t0, 0
  sw t0, 96(sp)
  flw ft0, 0(sp)
  fsw ft0, 100(sp)
  lw t0, 96(sp)
  sw t0, 104(sp)
  lw t0, 8(sp)
  sw t0, 108(sp)
  lw a0, 100(sp)
  lw a1, 104(sp)
  lw a2, 108(sp)
  call swap
  sw a0, 112(sp)
  lw t0, 112(sp)
  sw t0, 12(sp)
  lw t0, 8(sp)
  sw t0, 116(sp)
  lw t0, 116(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 120(sp)
  lw t0, 120(sp)
  sw t0, 12(sp)
  flw ft0, 0(sp)
  fsw ft0, 124(sp)
  lw t0, 96(sp)
  sw t0, 128(sp)
  lw t0, 12(sp)
  sw t0, 132(sp)
  lw a0, 124(sp)
  lw a1, 128(sp)
  lw a2, 132(sp)
  call heap_ajust
  sw a0, 136(sp)
  lw t0, 136(sp)
  sw t0, 12(sp)
  lw t0, 8(sp)
  sw t0, 140(sp)
  lw t0, 140(sp)
  li t1, 1
  sub t0, t0, t1
  sw t0, 144(sp)
  lw t0, 144(sp)
  sw t0, 8(sp)
  j heap_sort_while_entry_3
heap_sort_while_end_5:
  li a0, 0
  lw ra, 156(sp)
  addi sp, sp, 160
  ret
  .text
  .globl main
main:
  addi sp, sp, -160
  sw ra, 156(sp)
main_entry:
  li t0, 10
  la t1, n
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 40(sp)
  li t0, 4
  fcvt.s.w ft0, t0
  lw t1, 40(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 1
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 44(sp)
  li t0, 3
  fcvt.s.w ft0, t0
  lw t1, 44(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 48(sp)
  li t0, 9
  fcvt.s.w ft0, t0
  lw t1, 48(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 3
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 52(sp)
  li t0, 2
  fcvt.s.w ft0, t0
  lw t1, 52(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 4
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 56(sp)
  li t0, 0
  fcvt.s.w ft0, t0
  lw t1, 56(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 5
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 60(sp)
  li t0, 1
  fcvt.s.w ft0, t0
  lw t1, 60(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 6
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 64(sp)
  li t0, 6
  fcvt.s.w ft0, t0
  lw t1, 64(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 7
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 68(sp)
  li t0, 5
  fcvt.s.w ft0, t0
  lw t1, 68(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 8
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 72(sp)
  li t0, 7
  fcvt.s.w ft0, t0
  lw t1, 72(sp)
  fsw ft0, 0(t1)
  addi t0, sp, 0
  li t1, 9
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 76(sp)
  li t0, 8
  fcvt.s.w ft0, t0
  lw t1, 76(sp)
  fsw ft0, 0(t1)
  li t0, 0
  sw t0, 80(sp)
  addi t0, sp, 0
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 84(sp)
  la t1, n
  lw t0, 0(t1)
  sw t0, 88(sp)
  lw a0, 84(sp)
  lw a1, 88(sp)
  call heap_sort
  sw a0, 92(sp)
  lw t0, 92(sp)
  sw t0, 80(sp)
  j main_while_entry_0
main_while_entry_0:
  lw t0, 80(sp)
  sw t0, 96(sp)
  la t1, n
  lw t0, 0(t1)
  sw t0, 100(sp)
  lw t0, 96(sp)
  lw t1, 100(sp)
  slt t0, t0, t1
  sw t0, 104(sp)
  lw t0, 104(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 108(sp)
  lw t0, 108(sp)
  bnez t0, main_while_body_1
  j main_while_end_2
main_while_body_1:
  lw t0, 80(sp)
  sw t0, 116(sp)
  addi t0, sp, 0
  lw t1, 116(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 120(sp)
  lw t1, 120(sp)
  flw ft0, 0(t1)
  fsw ft0, 124(sp)
  flw ft0, 124(sp)
  fcvt.w.s t0, ft0, rtz
  sw t0, 112(sp)
  lw t0, 112(sp)
  sw t0, 128(sp)
  lw a0, 128(sp)
  call putint
  li t0, 10
  sw t0, 112(sp)
  lw t0, 112(sp)
  sw t0, 132(sp)
  lw a0, 132(sp)
  call putch
  lw t0, 80(sp)
  sw t0, 136(sp)
  lw t0, 136(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 140(sp)
  lw t0, 140(sp)
  sw t0, 80(sp)
  j main_while_entry_0
main_while_end_2:
  li a0, 0
  lw ra, 156(sp)
  addi sp, sp, 160
  ret
