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
    }
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

        stack[sp] = Nan_Box(inst.operand);
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

        stack[sp-2] /= stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_JMP:
        // no need to check for negative addrs
        if (inst.operand.as_ptr() >= (void*)current_program_size)
            return Exception_Type::EXCEPTION_INVALID_JMP_ADDR;

        ip = (uint64_t)inst.operand.as_ptr();
        return Exception_Type::EXCEPTION_OK;

    case Inst_Type::INST_HALT:
        // nothing to do
        return Exception_Type::EXCEPTION_OK;

    case Inst_Type::INST_EXIT:
        return Exception_Type::EXCEPTION_EXIT;

    case Inst_Type::INST_EQU:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        // stack[sp-2] = stack[sp-2] == stack[sp-1];
        stack[sp-2].box_int(stack[sp-2] == stack[sp-1]);
        sp--;
        break;

    case Inst_Type::INST_JMP_IF:
        if (stack[sp-1] == Nan_Box(1)) {
            ip = (uint64_t)inst.operand.as_ptr();
            return Exception_Type::EXCEPTION_OK;
        }
        sp--;
        break;

    case Inst_Type::INST_DUP:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp >= STACK_CAP)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        if (sp-inst.operand.as_int() <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-inst.operand.as_int() > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        stack[sp] = stack[sp-inst.operand.as_int()-1];
        sp++;
        break;

    case Inst_Type::INST_DUMP:
        this->dump_stack();
        break;

    case Inst_Type::INST_SWAP:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp >= STACK_CAP)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        if (sp-inst.operand.as_int() <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-inst.operand.as_int() > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        std::swap(stack[sp-1], stack[sp-inst.operand.as_int()-1]);
        break;

    case Inst_Type::INST_CALL:
        if (sp >= STACK_CAP)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        if (inst.operand.as_ptr() >= (void*)current_program_size)
            return Exception_Type::EXCEPTION_INVALID_JMP_ADDR;

        stack[sp] = Nan_Box((void*)(ip+1));
        ip = (uint64_t)inst.operand.as_ptr();
        sp++;
        return Exception_Type::EXCEPTION_OK;

    case Inst_Type::INST_RET:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (stack[sp-1].get_type() != Nan_Type::PTR)
            return Exception_Type::EXCEPTION_INVALID_RET_ADDR;

        ip = (uint64_t)stack[sp-1].as_ptr();
        sp--;
        return Exception_Type::EXCEPTION_OK;

    case Inst_Type::INST_NATIVE:
        // we assume that all the native functions return exactly one value
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        // call the native functions
        (this->*native_funcs_addrs[inst.operand.as_int()])();
        break;

    case Inst_Type::INST_PRINT:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-inst.operand.as_int() <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-inst.operand.as_int() > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        switch (stack[sp-inst.operand.as_int()-1].get_type()) {
        case Nan_Type::DOUBLE:
            std::cout << stack[sp-inst.operand.as_int()-1].as_double() << std::endl;
            break;

        case Nan_Type::INT:
            std::cout << stack[sp-inst.operand.as_int()-1].as_int() << std::endl;
            break;

        case Nan_Type::PTR:
            std::cout << stack[sp-inst.operand.as_int()-1].as_ptr() << std::endl;
            break;

        case Nan_Type::EXCEPTION:
        default:
            std::cerr << "ERROR: Unknown variable data type in the stack." << std::endl;
            exit(1);
        }
        break;

    case Inst_Type::INST_COUNT:
    default:
        return Exception_Type::EXCEPTION_UNKNOWN_INSTRUCTION;
    }

    ip++;

    // check for exceptions in the nan types
    if (stack[sp-1].get_type() == Nan_Type::EXCEPTION)
        return stack[sp-1].as_exception();

    // nothing went wrong
    return Exception_Type::EXCEPTION_OK;
}

void Vm::dump_stack() {
    // print the stack
    std::cout << "Vm Stack (" << sp << " element" << (sp != 1 ? "s" : "") << "):" << std::endl;
    for (size_t i = 0; i < sp; i++) {
        switch (stack[i].get_type()) {
        case Nan_Type::DOUBLE:
            std::cout << "    # Double: " << stack[i].as_double() << std::endl;
            break;

        case Nan_Type::INT:
            std::cout << "    # Int: " << stack[i].as_int() << std::endl;
            break;

        case Nan_Type::PTR:
            std::cout << "    # Ptr: " << stack[i].as_ptr() << std::endl;
            break;

        case Nan_Type::EXCEPTION:
        default:
            std::cerr << "ERROR: Unknown variable data type in the stack." << std::endl;
            exit(1);
        }
    }

    if (sp == 0)
        std::cout << "    [Empty]" << std::endl;
}

// native functions
// WARNING: we assume that the state of the machine was checked before calling this native functions

void Vm::native_malloc() {
    // get the size of the memory to allocate
    const size_t size = stack[sp-1].as_int();

    // allocate the memory
    const void *const ptr = malloc(size);

    // store the pointer in the stack
    stack[sp-1].box_ptr(ptr);
}

void Vm::native_free() {
    // get the pointer to free
    void *const ptr = stack[sp-1].as_ptr();

    // free the memory
    free(ptr);

    // comsume the addr
    sp--;
}
