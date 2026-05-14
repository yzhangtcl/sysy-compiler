gcc ./test/$1.c -o "./test/$1_c"
"./test/$1_c"
val1=$?
echo "gcc return value: $val1"
./build/compiler -riscv "./test/$1.c" -o "./test/$1.s"
riscv64-linux-gnu-gcc "./test/$1.s" -o "./test/$1_riscv"
qemu-riscv64 -L /usr/riscv64-linux-gnu "./test/$1_riscv"
val2=$?
echo "riscv return value: $val2"