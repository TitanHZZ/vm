#!/usr/bin/bash

set -xe

/usr/bin/nasm -felf64 template.asm
/usr/bin/ld -o template template.o
