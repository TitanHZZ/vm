#!/usr/bin/bash

set -xe

# compile
/usr/bin/nasm -f elf64 nan_box.asm -o nan_box.o
/usr/bin/nasm -f elf64 template.asm -o template.o

# link
/usr/bin/ld -m elf_x86_64 template.o nan_box.o -o template
