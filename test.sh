gcc -include ./sysy-runtime-lib/src/sysy.h ./sysy-runtime-lib/src/sysy.c ./test/$1.c  -o "./test/$1_c" 
"./test/$1_c"
val1=$?
echo "gcc return value: $val1"
./build/compiler -riscv "./test/$1.c" -o "./test/$1.s"
clang "./test/$1.s" -c -o "./test/$1.o" -target riscv32-unknown-linux-elf -march=rv32imf -mabi=ilp32f
ld.lld -static "./test/$1.o" -L ./sysy-runtime-lib/build -lsysy -o "./test/$1_riscv32"
qemu-riscv32 "./test/$1_riscv32"
val2=$?
echo "riscv32 return value: $val2"