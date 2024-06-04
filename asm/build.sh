#!/usr/bin/bash

set -xe

# compile
/usr/bin/nasm -f elf64 nan_box.asm -o nan_box.o
/usr/bin/nasm -f elf64 template.asm -o template.o

# link
/usr/bin/gcc -m64 -no-pie -o template template.o nan_box.o -nostartfiles
