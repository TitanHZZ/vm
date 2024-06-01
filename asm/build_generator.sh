#!/usr/bin/bash

set -xe

../build/src/vasm2x86_64 -i ../pre.vm > gen.asm

/usr/bin/nasm -felf64 gen.asm
/usr/bin/ld -o gen gen.o
