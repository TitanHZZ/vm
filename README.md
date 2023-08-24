# vm
Simple vm implementation.

## Quick Start
Before building and using this project, you will need to install [cmake](https://cmake.org/).

```console
$ mkdir build
$ cmake --build ./build --target all
$ ./build/vasma -i examples/fibonacci.vasm -o fib.vm
$ ./build/vme -i fib.vm
```

## Components

### vasm

Assembly language for the virtual machine.  
For examples look in the [examples](https://github.com/TitanHZZ/vm/tree/main/examples) folder.

### vm

Byte code (executable) file format, compiled with [vasma](#vasma).

### vasma

'vasma' is the vasm assembler that generates byte code from a [source file](https://github.com/TitanHZZ/vm/tree/main/examples) to be ran by [vme](#vme).

### vme

The vasm executor, used to run [vm](#vm-1) files.
