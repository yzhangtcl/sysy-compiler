  .text
  .globl mean
mean:
  addi sp, sp, -80
mean_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  li t0, 0
  fmv.w.x ft0, t0
  fsw ft0, 16(sp)
  flw ft0, 16(sp)
  fsw ft0, 8(sp)
  li t0, 0
  sw t0, 12(sp)
  j mean_while_entry_0
mean_while_entry_0:
  lw t0, 12(sp)
  sw t0, 20(sp)
  lw t0, 4(sp)
  sw t0, 24(sp)
  lw t0, 20(sp)
  lw t1, 24(sp)
  slt t0, t0, t1
  sw t0, 28(sp)
  lw t0, 28(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 32(sp)
  lw t0, 32(sp)
  bnez t0, mean_while_body_1
  j mean_while_end_2
mean_while_body_1:
  flw ft0, 8(sp)
  fsw ft0, 36(sp)
  flw ft0, 0(sp)
  fsw ft0, 40(sp)
  lw t0, 12(sp)
  sw t0, 44(sp)
  lw t0, 40(sp)
  lw t1, 44(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 48(sp)
  lw t1, 48(sp)
  flw ft0, 0(t1)
  fsw ft0, 52(sp)
  flw ft0, 36(sp)
  flw ft1, 52(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 56(sp)
  flw ft0, 56(sp)
  fsw ft0, 8(sp)
  lw t0, 12(sp)
  sw t0, 60(sp)
  lw t0, 60(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 64(sp)
  lw t0, 64(sp)
  sw t0, 12(sp)
  j mean_while_entry_0
mean_while_end_2:
  flw ft0, 8(sp)
  fsw ft0, 68(sp)
  lw t0, 4(sp)
  sw t0, 72(sp)
  flw ft0, 68(sp)
  lw t0, 72(sp)
  fcvt.s.w ft1, t0
  fdiv.s ft0, ft0, ft1
  fsw ft0, 76(sp)
  flw fa0, 76(sp)
  addi sp, sp, 80
  ret
  .text
  .globl variance
variance:
  addi sp, sp, -112
variance_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  mv t0, a2
  fmv.w.x ft0, t0
  fsw ft0, 8(sp)
  li t0, 0
  fmv.w.x ft0, t0
  fsw ft0, 24(sp)
  flw ft0, 24(sp)
  fsw ft0, 12(sp)
  li t0, 0
  sw t0, 20(sp)
  j variance_while_entry_0
variance_while_entry_0:
  lw t0, 20(sp)
  sw t0, 28(sp)
  lw t0, 4(sp)
  sw t0, 32(sp)
  lw t0, 28(sp)
  lw t1, 32(sp)
  slt t0, t0, t1
  sw t0, 36(sp)
  lw t0, 36(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 40(sp)
  lw t0, 40(sp)
  bnez t0, variance_while_body_1
  j variance_while_end_2
variance_while_body_1:
  flw ft0, 0(sp)
  fsw ft0, 44(sp)
  lw t0, 20(sp)
  sw t0, 48(sp)
  lw t0, 44(sp)
  lw t1, 48(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 52(sp)
  lw t1, 52(sp)
  flw ft0, 0(t1)
  fsw ft0, 56(sp)
  flw ft0, 8(sp)
  fsw ft0, 60(sp)
  flw ft0, 56(sp)
  flw ft1, 60(sp)
  fsub.s ft0, ft0, ft1
  fsw ft0, 64(sp)
  flw ft0, 64(sp)
  fsw ft0, 16(sp)
  flw ft0, 12(sp)
  fsw ft0, 68(sp)
  flw ft0, 16(sp)
  fsw ft0, 72(sp)
  flw ft0, 16(sp)
  fsw ft0, 76(sp)
  flw ft0, 72(sp)
  flw ft1, 76(sp)
  fmul.s ft0, ft0, ft1
  fsw ft0, 80(sp)
  flw ft0, 68(sp)
  flw ft1, 80(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 84(sp)
  flw ft0, 84(sp)
  fsw ft0, 12(sp)
  lw t0, 20(sp)
  sw t0, 88(sp)
  lw t0, 88(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 92(sp)
  lw t0, 92(sp)
  sw t0, 20(sp)
  j variance_while_entry_0
variance_while_end_2:
  flw ft0, 12(sp)
  fsw ft0, 96(sp)
  lw t0, 4(sp)
  sw t0, 100(sp)
  flw ft0, 96(sp)
  lw t0, 100(sp)
  fcvt.s.w ft1, t0
  fdiv.s ft0, ft0, ft1
  fsw ft0, 104(sp)
  flw fa0, 104(sp)
  addi sp, sp, 112
  ret
  .text
  .globl covariance
covariance:
  addi sp, sp, -128
covariance_entry:
  mv t0, a0
  sw t0, 0(sp)
  mv t0, a1
  sw t0, 4(sp)
  mv t0, a2
  sw t0, 8(sp)
  mv t0, a3
  fmv.w.x ft0, t0
  fsw ft0, 12(sp)
  mv t0, a4
  fmv.w.x ft0, t0
  fsw ft0, 16(sp)
  li t0, 0
  fmv.w.x ft0, t0
  fsw ft0, 28(sp)
  flw ft0, 28(sp)
  fsw ft0, 20(sp)
  li t0, 0
  sw t0, 24(sp)
  j covariance_while_entry_0
covariance_while_entry_0:
  lw t0, 24(sp)
  sw t0, 32(sp)
  lw t0, 8(sp)
  sw t0, 36(sp)
  lw t0, 32(sp)
  lw t1, 36(sp)
  slt t0, t0, t1
  sw t0, 40(sp)
  lw t0, 40(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 44(sp)
  lw t0, 44(sp)
  bnez t0, covariance_while_body_1
  j covariance_while_end_2
covariance_while_body_1:
  flw ft0, 20(sp)
  fsw ft0, 48(sp)
  flw ft0, 0(sp)
  fsw ft0, 52(sp)
  lw t0, 24(sp)
  sw t0, 56(sp)
  lw t0, 52(sp)
  lw t1, 56(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 60(sp)
  lw t1, 60(sp)
  flw ft0, 0(t1)
  fsw ft0, 64(sp)
  flw ft0, 12(sp)
  fsw ft0, 68(sp)
  flw ft0, 64(sp)
  flw ft1, 68(sp)
  fsub.s ft0, ft0, ft1
  fsw ft0, 72(sp)
  flw ft0, 4(sp)
  fsw ft0, 76(sp)
  lw t0, 24(sp)
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
  flw ft0, 16(sp)
  fsw ft0, 92(sp)
  flw ft0, 88(sp)
  flw ft1, 92(sp)
  fsub.s ft0, ft0, ft1
  fsw ft0, 96(sp)
  flw ft0, 72(sp)
  flw ft1, 96(sp)
  fmul.s ft0, ft0, ft1
  fsw ft0, 100(sp)
  flw ft0, 48(sp)
  flw ft1, 100(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 104(sp)
  flw ft0, 104(sp)
  fsw ft0, 20(sp)
  lw t0, 24(sp)
  sw t0, 108(sp)
  lw t0, 108(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 112(sp)
  lw t0, 112(sp)
  sw t0, 24(sp)
  j covariance_while_entry_0
covariance_while_end_2:
  flw ft0, 20(sp)
  fsw ft0, 116(sp)
  lw t0, 8(sp)
  sw t0, 120(sp)
  flw ft0, 116(sp)
  lw t0, 120(sp)
  fcvt.s.w ft1, t0
  fdiv.s ft0, ft0, ft1
  fsw ft0, 124(sp)
  flw fa0, 124(sp)
  addi sp, sp, 128
  ret
  .text
  .globl main
main:
  addi sp, sp, -1104
  sw ra, 1100(sp)
main_entry:
  call getint
  sw a0, 840(sp)
  lw t0, 840(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 844(sp)
  lw t0, 844(sp)
  li t1, 0
  sgt t0, t0, t1
  seqz t0, t0
  sw t0, 848(sp)
  lw t0, 848(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 852(sp)
  lw t0, 852(sp)
  bnez t0, main_if_then_0
  j main_if_end_1
main_if_then_0:
  li t0, 0
  li t1, 1
  sub t0, t0, t1
  sw t0, 856(sp)
  lw a0, 856(sp)
  call putint
  li t0, 0
  li t1, 1
  sub t0, t0, t1
  sw t0, 860(sp)
  lw a0, 860(sp)
  lw ra, 1100(sp)
  addi sp, sp, 1104
  ret
main_if_end_1:
  lw t0, 0(sp)
  sw t0, 864(sp)
  lw t0, 864(sp)
  li t1, 100
  sgt t0, t0, t1
  sw t0, 868(sp)
  lw t0, 868(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 872(sp)
  lw t0, 872(sp)
  bnez t0, main_if_then_2
  j main_if_end_3
main_if_then_2:
  li t0, 0
  li t1, 2
  sub t0, t0, t1
  sw t0, 876(sp)
  lw a0, 876(sp)
  call putint
  li t0, 0
  li t1, 2
  sub t0, t0, t1
  sw t0, 880(sp)
  lw a0, 880(sp)
  lw ra, 1100(sp)
  addi sp, sp, 1104
  ret
main_if_end_3:
  li t0, 0
  sw t0, 836(sp)
  j main_while_entry_4
main_while_entry_4:
  lw t0, 836(sp)
  sw t0, 884(sp)
  lw t0, 0(sp)
  sw t0, 888(sp)
  lw t0, 884(sp)
  lw t1, 888(sp)
  slt t0, t0, t1
  sw t0, 892(sp)
  lw t0, 892(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 896(sp)
  lw t0, 896(sp)
  bnez t0, main_while_body_5
  j main_while_end_6
main_while_body_5:
  call getfloat
  fsw fa0, 900(sp)
  lw t0, 836(sp)
  sw t0, 904(sp)
  addi t0, sp, 4
  lw t1, 904(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 908(sp)
  flw ft0, 900(sp)
  lw t1, 908(sp)
  fsw ft0, 0(t1)
  call getfloat
  fsw fa0, 912(sp)
  lw t0, 836(sp)
  sw t0, 916(sp)
  addi t0, sp, 404
  lw t1, 916(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 920(sp)
  flw ft0, 912(sp)
  lw t1, 920(sp)
  fsw ft0, 0(t1)
  lw t0, 836(sp)
  sw t0, 924(sp)
  lw t0, 924(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 928(sp)
  lw t0, 928(sp)
  sw t0, 836(sp)
  j main_while_entry_4
main_while_end_6:
  addi t0, sp, 4
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 932(sp)
  lw t0, 0(sp)
  sw t0, 936(sp)
  lw a0, 932(sp)
  lw a1, 936(sp)
  call mean
  fsw fa0, 940(sp)
  flw ft0, 940(sp)
  fsw ft0, 804(sp)
  addi t0, sp, 404
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 944(sp)
  lw t0, 0(sp)
  sw t0, 948(sp)
  lw a0, 944(sp)
  lw a1, 948(sp)
  call mean
  fsw fa0, 952(sp)
  flw ft0, 952(sp)
  fsw ft0, 808(sp)
  addi t0, sp, 4
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 956(sp)
  lw t0, 0(sp)
  sw t0, 960(sp)
  flw ft0, 804(sp)
  fsw ft0, 964(sp)
  lw a0, 956(sp)
  lw a1, 960(sp)
  lw a2, 964(sp)
  call variance
  fsw fa0, 968(sp)
  flw ft0, 968(sp)
  fsw ft0, 812(sp)
  addi t0, sp, 4
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 972(sp)
  addi t0, sp, 404
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 976(sp)
  lw t0, 0(sp)
  sw t0, 980(sp)
  flw ft0, 804(sp)
  fsw ft0, 984(sp)
  flw ft0, 808(sp)
  fsw ft0, 988(sp)
  lw a0, 972(sp)
  lw a1, 976(sp)
  lw a2, 980(sp)
  lw a3, 984(sp)
  lw a4, 988(sp)
  call covariance
  fsw fa0, 992(sp)
  flw ft0, 992(sp)
  fsw ft0, 816(sp)
  flw ft0, 812(sp)
  fsw ft0, 996(sp)
  li t0, 0
  fmv.w.x ft0, t0
  fsw ft0, 1000(sp)
  flw ft0, 996(sp)
  flw ft1, 1000(sp)
  feq.s t0, ft0, ft1
  sw t0, 1004(sp)
  lw t0, 1004(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 1008(sp)
  lw t0, 1008(sp)
  bnez t0, main_if_then_7
  j main_if_end_8
main_if_then_7:
  li t0, 0
  li t1, 3
  sub t0, t0, t1
  sw t0, 1012(sp)
  lw a0, 1012(sp)
  call putint
  li t0, 0
  li t1, 3
  sub t0, t0, t1
  sw t0, 1016(sp)
  lw a0, 1016(sp)
  lw ra, 1100(sp)
  addi sp, sp, 1104
  ret
main_if_end_8:
  flw ft0, 816(sp)
  fsw ft0, 1020(sp)
  flw ft0, 812(sp)
  fsw ft0, 1024(sp)
  flw ft0, 1020(sp)
  flw ft1, 1024(sp)
  fdiv.s ft0, ft0, ft1
  fsw ft0, 1028(sp)
  flw ft0, 1028(sp)
  fsw ft0, 820(sp)
  flw ft0, 808(sp)
  fsw ft0, 1032(sp)
  flw ft0, 820(sp)
  fsw ft0, 1036(sp)
  flw ft0, 804(sp)
  fsw ft0, 1040(sp)
  flw ft0, 1036(sp)
  flw ft1, 1040(sp)
  fmul.s ft0, ft0, ft1
  fsw ft0, 1044(sp)
  flw ft0, 1032(sp)
  flw ft1, 1044(sp)
  fsub.s ft0, ft0, ft1
  fsw ft0, 1048(sp)
  flw ft0, 1048(sp)
  fsw ft0, 824(sp)
  flw ft0, 820(sp)
  fsw ft0, 1052(sp)
  flw fa0, 1052(sp)
  call putfloat
  flw ft0, 824(sp)
  fsw ft0, 1056(sp)
  flw fa0, 1056(sp)
  call putfloat
  call getfloat
  fsw fa0, 1060(sp)
  flw ft0, 1060(sp)
  fsw ft0, 828(sp)
  flw ft0, 820(sp)
  fsw ft0, 1064(sp)
  flw ft0, 828(sp)
  fsw ft0, 1068(sp)
  flw ft0, 1064(sp)
  flw ft1, 1068(sp)
  fmul.s ft0, ft0, ft1
  fsw ft0, 1072(sp)
  flw ft0, 824(sp)
  fsw ft0, 1076(sp)
  flw ft0, 1072(sp)
  flw ft1, 1076(sp)
  fadd.s ft0, ft0, ft1
  fsw ft0, 1080(sp)
  flw ft0, 1080(sp)
  fsw ft0, 832(sp)
  flw ft0, 832(sp)
  fsw ft0, 1084(sp)
  flw fa0, 1084(sp)
  call putfloat
  li a0, 0
  lw ra, 1100(sp)
  addi sp, sp, 1104
  ret
