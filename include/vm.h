#pragma once
#include <iostream>
#include <vector>

#include "exceptions.h"
#include "program.h"
#include "inst.h"
#include "nan_box.h"

#define STACK_CAP 1024
#define DEFAULT_STATIC_MEM_CAP 300
#define EXECUTION_LIMIT 10000

class Vm {
private:
    // stack
    // TODO: replace the stack with a vector
    Nan_Box stack[STACK_CAP];
    size_t sp;

    // program
    uint64_t ip;
    size_t current_program_size;

    // static memory
    std::vector<Nan_Box> *memory;

    // native functions
    typedef void (Vm::*Native_Func)();
    void native_malloc();
    void native_free();
    void native_fwrite();

    constexpr static const Native_Func native_funcs_addrs[] = {
        &Vm::native_malloc,
        &Vm::native_free,
        &Vm::native_fwrite
    };

public:
    Vm();
    ~Vm();

    // void execute_program(Inst program[], const size_t program_size);
    void execute_program(const char *path);
    void execute_program(Program& program); /*{
        this->execute_program(program.insts.data(), program.insts.size());
    }*/

    Exception_Type execute_instruction(Inst& inst);

    // debug functions
    void dump_stack();
    void dump_memory();

    constexpr static std::string_view native_funcs_names[] = {
        "malloc",
        "free",
        "fwrite"
    };

    constexpr static size_t native_funcs_count = sizeof(native_funcs_addrs) / sizeof(Native_Func);
};
