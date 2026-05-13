  .text
  .globl main
main:
  addi sp, sp, -48
  li t0, 11
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 0(sp)
  li t0, 21
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 4(sp)
  lw t0, 0(sp)
  lw t1, 4(sp)
  and t0, t0, t1
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 12(sp)
  li t0, 40
  li t1, 3
  div t0, t0, t1
  sw t0, 16(sp)
  lw t0, 16(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 20(sp)
  lw t0, 12(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 24(sp)
  lw t0, 20(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 28(sp)
  lw t0, 24(sp)
  lw t1, 28(sp)
  or t0, t0, t1
  sw t0, 32(sp)
  lw t0, 32(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 36(sp)
  lw a0, 36(sp)
  addi sp, sp, 48
  ret
