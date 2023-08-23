#include <iostream>

#include "vm.h"

Vm::Vm() {}
Vm::~Vm(){}

void Vm::execute_program(Inst program[], const size_t program_size) {
    // check for empty program
    if (program_size == 0) {
        std::cout << "WARNING: Ignoring empty program!" << std::endl;
        return;
    }

    // reset the vm
    ip = 0;
    sp = 0;
    current_program_size = program_size;

    for (size_t i = 0; i < EXECUTION_LIMIT && ip < current_program_size; i++) {
        const Exception_Type exception = this->execute_instruction(program[ip]);

        if (exception == Exception_Type::EXCEPTION_EXIT)
            break;

        if (exception != Exception_Type::EXCEPTION_OK) {
            exception_handler(exception);
            exit(1);
        }

        this->dump_stack(); // debug
    }

    std::cout << "Program Terminated." << std::endl;
}

void Vm::execute_program(const char *path) {
    Program program;
    program.parse_from_file(path);
    this->execute_program(program);
}

Exception_Type Vm::execute_instruction(Inst& inst) {
    // check for stack overflow
    if (sp >= STACK_CAP) {
        std::cerr << "ERROR: Stack Overflow in '" << __func__ << "'" << std::endl;
        exit(1);
    }

    switch (inst.type) {
    case Inst_Type::INST_NOP:
        break;

    case Inst_Type::INST_PUSH:
        if (sp >= STACK_CAP)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        stack[sp] = inst.operand;
        sp++;
        break;

    case Inst_Type::INST_POP:
        if (sp == 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        sp--;
        break;

    case Inst_Type::INST_ADD:
        if (sp < 2)
                return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] += stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_SUB:
        if (sp < 2)
                return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] -= stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_MUL:
        if (sp < 2)
                return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] *= stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_DIV:
        if (sp < 2)
                return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (inst.operand == 0)
            return Exception_Type::EXCEPTION_DIV_BY_ZERO;

        stack[sp-2] /= stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_JMP:
        if (sp < 1)
                return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        // no need to check for negative addrs because Word is unsigned
        if (inst.operand >= current_program_size)
            return Exception_Type::EXCEPTION_INVALID_JMP_ADDR;

        ip = inst.operand;
        return Exception_Type::EXCEPTION_OK;

    case Inst_Type::INST_HALT:
        // nothing to do
        return Exception_Type::EXCEPTION_OK;

    case Inst_Type::INST_EXIT:
        return Exception_Type::EXCEPTION_EXIT;

    case Inst_Type::INST_EQU:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] = stack[sp-2] == stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_JMP_IF:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (stack[sp-1] == 1) {
            ip = inst.operand;
            return Exception_Type::EXCEPTION_OK;
        }
        sp--;
        break;

    case Inst_Type::INST_DUP:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp >= STACK_CAP)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        stack[sp] = stack[sp-inst.operand-1];
        sp++;
        break;

    case Inst_Type::INST_COUNT:
    default:
        return Exception_Type::EXCEPTION_UNKNOWN_INSTRUCTION;
    }

    ip++;

    // nothing went wrong
    return Exception_Type::EXCEPTION_OK;
}

void Vm::dump_stack() {
    // print the stack
    std::cout << "Vm Stack (" << sp << " element" << (sp != 1 ? "s" : "") << "):" << std::endl;
    for (size_t i = 0; i < sp; i++) {
        std::cout << "    # " << stack[i] << std::endl;
    }

    if (sp == 0)
        std::cout << "    [Empty]" << std::endl;
}
