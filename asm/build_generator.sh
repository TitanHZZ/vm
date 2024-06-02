#!/usr/bin/bash

set -xe

../build/src/vasma -i ../examples/123i.vasm -o ../build/123i.vm
../build/src/vasm2x86_64 -i ../build/123i.vm > gen.asm

# compile
/usr/bin/nasm -f elf64 nan_box.asm -o nan_box.o
/usr/bin/nasm -f elf64 gen.asm -o gen.o

# link
/usr/bin/ld -m elf_x86_64 gen.o nan_box.o -o gen
