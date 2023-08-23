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
