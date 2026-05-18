  .text
  .globl main
main:
  addi sp, sp, -48
  sw ra, 44(sp)
main_entry:
  call getfloat
  fsw fa0, 4(sp)
  flw ft0, 4(sp)
  fsw ft0, 0(sp)
  flw ft0, 0(sp)
  fsw ft0, 8(sp)
  flw ft0, 8(sp)
  fcvt.w.s a0, ft0, rtz
  call putint
  li a0, 10
  call putch
  flw ft0, 0(sp)
  fsw ft0, 12(sp)
  flw fa0, 12(sp)
  call putfloat
  li a0, 10
  call putch
  flw ft0, 0(sp)
  fsw ft0, 20(sp)
  flw ft0, 20(sp)
  fcvt.w.s t0, ft0, rtz
  sw t0, 16(sp)
  lw t0, 16(sp)
  sw t0, 24(sp)
  lw a0, 24(sp)
  call putint
  li a0, 10
  call putch
  lw t0, 16(sp)
  sw t0, 28(sp)
  lw t0, 28(sp)
  fcvt.s.w fa0, t0
  call putfloat
  li a0, 10
  call putch
  lw t0, 16(sp)
  sw t0, 32(sp)
  lw a0, 32(sp)
  lw ra, 44(sp)
  addi sp, sp, 48
  ret
