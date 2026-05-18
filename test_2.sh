gcc -include ./sysy-runtime-lib/src/sysy.h ./sysy-runtime-lib/src/sysy.c ./test/$1.c  -o "./test/$1_c" 
"./test/$1_c" < "./test/$1.in" > "./test/$1_c.out"
val1=$?
echo "gcc output:--------------"
cat "./test/$1_c.out"
echo ""
echo "return value: $val1"
echo "-------------------------"
./build/compiler -riscv "./test/$1.c" -o "./test/$1.s" 
riscv64-linux-gnu-gcc "./test/$1.s" -o "./test/$1_riscv" -L ./sysy-runtime-lib/build -lsysy -static
qemu-riscv64 -L /usr/riscv64-linux-gnu "./test/$1_riscv" < "./test/$1.in" > "./test/$1_riscv.out"
val2=$?
echo "riscv output:--------------"
cat "./test/$1_riscv.out"
echo ""
echo "return value: $val2"
echo "-------------------------"
