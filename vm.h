#pragma once
#include <iostream>

#include "exceptions.h"
#include "program.h"
#include "inst.h"
#include "nan_box.h"

#define STACK_CAP 1024
#define EXECUTION_LIMIT 10000

class Vm {
private:
    typedef void (Vm::*Native_Func)();

    Nan_Box stack[STACK_CAP];
    size_t sp;

    uint64_t ip;
    size_t current_program_size;

    // native functions
    void native_malloc();
    void native_free();

    constexpr static const Native_Func native_funcs_addrs[] = {
        &Vm::native_malloc,
        &Vm::native_free
    };

public:
    Vm();
    ~Vm();

    void execute_program(Inst program[], const size_t program_size);
    void execute_program(const char *path);
    inline void execute_program(Program& program) {
        this->execute_program(program.insts.data(), program.insts.size());
    }

    Exception_Type execute_instruction(Inst& inst);

    // debug functions
    void dump_stack();

    constexpr static std::string_view native_funcs_names[] = {
        "malloc",
        "free"
    };

    constexpr static size_t native_funcs_count = sizeof(native_funcs_addrs) / sizeof(Native_Func);
};
