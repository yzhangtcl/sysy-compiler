gcc -include ./sysy-runtime-lib/src/sysy.h ./sysy-runtime-lib/src/sysy.c ./test/$1.c  -o "./test/$1_c" 
"./test/$1_c" < "./test/$1.in" > "./test/$1_c.out"
val1=$?
echo "gcc output:--------------"
cat "./test/$1_c.out"
echo ""
echo "return value: $val1"
echo "-------------------------"
./build/compiler -riscv "./test/$1.c" -o "./test/$1.s" 
clang "./test/$1.s" -c -o "./test/$1.o" -target riscv32-unknown-linux-elf -march=rv32imf -mabi=ilp32f
ld.lld -static "./test/$1.o" -L ./sysy-runtime-lib/build -lsysy -o "./test/$1_riscv32"
qemu-riscv32 "./test/$1_riscv32" < "./test/$1.in" > "./test/$1_riscv32.out"
val2=$?
echo "riscv32 output:--------------"
cat "./test/$1_riscv32.out"
echo ""
echo "return value: $val2"
echo "-------------------------"
