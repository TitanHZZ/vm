#!/usr/bin/bash

set -xe

../build/src/vasma -i ../examples/fibonacci.vasm -o ../build/fibonacci.vm
../build/src/vasm2x86_64 -i ../build/fibonacci.vm > gen.asm

# compile
/usr/bin/nasm -f elf64 nan_box.asm -o nan_box.o
/usr/bin/nasm -f elf64 gen.asm -o gen.o

# link
/usr/bin/gcc -m64 -no-pie -o gen gen.o nan_box.o -nostartfiles
