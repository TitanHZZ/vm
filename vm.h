#pragma once
#include <iostream>

#include "exceptions.h"
#include "program.h"
#include "inst.h"

#define STACK_CAP 1024
#define EXECUTION_LIMIT 100

class Vm {
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

private:
    Word stack[STACK_CAP];
    size_t sp;

    Word ip;
    size_t current_program_size;
};
