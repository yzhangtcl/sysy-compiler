  .text
  .globl main
main:
  addi sp, sp, -96
main_entry:
  addi t0, sp, 0
  li t1, 0
  li t2, 12
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 24(sp)
  ld t0, 24(sp)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 28(sp)
  li t0, 1
  ld t1, 28(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 0
  li t2, 12
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 32(sp)
  ld t0, 32(sp)
  li t1, 1
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 36(sp)
  li t0, 2
  ld t1, 36(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 0
  li t2, 12
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 40(sp)
  ld t0, 40(sp)
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 44(sp)
  li t0, 3
  ld t1, 44(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 1
  li t2, 12
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 48(sp)
  ld t0, 48(sp)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 52(sp)
  li t0, 4
  ld t1, 52(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 1
  li t2, 12
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 56(sp)
  ld t0, 56(sp)
  li t1, 1
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 60(sp)
  li t0, 5
  ld t1, 60(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 1
  li t2, 12
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 64(sp)
  ld t0, 64(sp)
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 68(sp)
  li t0, 6
  ld t1, 68(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 1
  li t2, 12
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 72(sp)
  ld t0, 72(sp)
  li t1, 2
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sd t0, 76(sp)
  ld t1, 76(sp)
  lw t0, 0(t1)
  sw t0, 80(sp)
  lw a0, 80(sp)
  addi sp, sp, 96
  ret
