make -j4
build/compiler -koopa test/3.c -o test/3.koopa
build/compiler -riscv test/3.c -o test/3.txt