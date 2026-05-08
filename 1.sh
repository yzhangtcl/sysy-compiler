make -j4
build/compiler -koopa test/1.c -o test/1.koopa
build/compiler -riscv test/1.c -o test/1.txt