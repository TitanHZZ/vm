#include <iostream>
#include <cstdint>

#include "vm.h"

void Vm::execute_program(const bool debug_mode) {
    // check for empty program
    if (program.insts.size() == 0) {
        std::cout << "WARNING: Ignoring empty program!" << std::endl;
        return;
    }

    // reset the vm
    ip = 0;
    sp = 0;
    current_program_size = program.insts.size();

    for (size_t i = 0; i < EXECUTION_LIMIT && ip < current_program_size && !debug_mode; i++) {
        const Exception_Type exception = execute_instruction(program.insts.at(ip));

        if (exception == Exception_Type::EXCEPTION_EXIT)
            break;

        if (exception != Exception_Type::EXCEPTION_OK) {
            exception_handler(exception);
            exit(1);
        }
    }
}

Exception_Type Vm::next() {
    return ip < current_program_size ? execute_instruction(program.insts.at(ip)) : Exception_Type::EXCEPTION_EXIT;
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

    case Inst_Type::INST_HALT:
        // nothing to do
        return Exception_Type::EXCEPTION_OK;

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

    case Inst_Type::INST_MOD:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] %= stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_JMP:
        // no need to check for negative addrs
        if (inst.operand.as_ptr() >= (void*)current_program_size)
            return Exception_Type::EXCEPTION_INVALID_JMP_ADDR;

        ip = (uint64_t)inst.operand.as_ptr();
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
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        sp--;
        if (stack[sp] != Nan_Box(static_cast<int64_t>(0))) {
            ip = (uint64_t)inst.operand.as_ptr();
            return Exception_Type::EXCEPTION_OK;
        }

        break;

    case Inst_Type::INST_DUP:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp >= STACK_CAP)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        stack[sp] = stack[sp-static_cast<size_t>(inst.operand.as_int())-1];
        sp++;
        break;

    case Inst_Type::INST_DUMP_STACK:
        dump_stack();
        break;

    case Inst_Type::INST_DUMP_MEMORY:
        dump_memory();
        break;

    case Inst_Type::INST_SWAP:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp >= STACK_CAP)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        std::swap(stack[sp-1], stack[sp-static_cast<size_t>(inst.operand.as_int())-1]);
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

        if (sp-static_cast<size_t>(inst.operand.as_int()) <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        switch (stack[sp-static_cast<size_t>(inst.operand.as_int())-1].get_type()) {
        case Nan_Type::DOUBLE:
            std::cout << stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_double() << std::endl;
            break;

        case Nan_Type::INT:
            std::cout << stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_int() << std::endl;
            break;

        case Nan_Type::PTR: {
            const void *const ptr = stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_ptr();
            const std::uintptr_t int_ptr = reinterpret_cast<std::uintptr_t>(ptr);
            std::cout << std::hex << std::showbase << int_ptr << std::endl;
            // taken from: https://www.tutorialspoint.com/cplusplus-program-to-print-values-in-a-specified-format
            // std::cout << std::hex << std::showbase << ptr << std::endl;
            // std::cout << std::hex << std::showbase << (long long) ptr << std::endl;
            // std::cout << stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_ptr() << std::endl;
            break;
        }

        case Nan_Type::EXCEPTION:
        default:
            std::cerr << "ERROR: Unknown variable data type in the stack." << std::endl;
            exit(1);
        }
        break;

    case Inst_Type::INST_TD:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        switch (stack[sp-static_cast<size_t>(inst.operand.as_int())-1].get_type()) {
        case Nan_Type::INT:{
            const double casted_value = (double)stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_int();
            stack[sp-static_cast<size_t>(inst.operand.as_int())-1].box_double(casted_value);
            break;
        }

        case Nan_Type::PTR: {
            const double casted_value = (double)(int64_t)stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_ptr();
            stack[sp-static_cast<size_t>(inst.operand.as_int())-1].box_double(casted_value);
            break;
        }

        case Nan_Type::DOUBLE:
            break;

        case Nan_Type::EXCEPTION:
        default:
            std::cerr << "ERROR: Unknown variable data type in the stack." << std::endl;
            exit(1);
        }
        break;

    case Inst_Type::INST_TI:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        switch (stack[sp-static_cast<size_t>(inst.operand.as_int())-1].get_type()) {
        case Nan_Type::INT:
            break;

        case Nan_Type::PTR: {
            const int64_t casted_value = (int64_t)stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_ptr();
            stack[sp-static_cast<size_t>(inst.operand.as_int())-1].box_int(casted_value);
            break;
        }

        case Nan_Type::DOUBLE: {
            const int64_t casted_value = (int64_t)stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_double();
            stack[sp-static_cast<size_t>(inst.operand.as_int())-1].box_int(casted_value);
            break;
        }

        case Nan_Type::EXCEPTION:
        default:
            std::cerr << "ERROR: Unknown variable data type in the stack." << std::endl;
            exit(1);
        }
        break;

    case Inst_Type::INST_TP:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) <= 0)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (sp-static_cast<size_t>(inst.operand.as_int()) > sp)
            return Exception_Type::EXCEPTION_STACK_OVERFLOW;

        switch (stack[sp-static_cast<size_t>(inst.operand.as_int())-1].get_type()) {
        case Nan_Type::INT: {
            const void *casted_value = (void *)stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_int();
            stack[sp-static_cast<size_t>(inst.operand.as_int())-1].box_ptr(casted_value);
            break;
        }

        case Nan_Type::PTR:
            break;

        case Nan_Type::DOUBLE: {
            const void *casted_value = (void *)(uint64_t)stack[sp-static_cast<size_t>(inst.operand.as_int())-1].as_double();
            stack[sp-static_cast<size_t>(inst.operand.as_int())-1].box_ptr(casted_value);
            break;
        }

        case Nan_Type::EXCEPTION:
        default:
            std::cerr << "ERROR: Unknown variable data type in the stack." << std::endl;
            exit(1);
        }
        break;

    case Inst_Type::INST_SHL:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] <<= stack[sp-1];
        sp--;
        break;

    case Inst_Type::INST_SHR: {
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        if (stack[sp-2].get_type() != Nan_Type::INT || stack[sp-1].get_type() != Nan_Type::INT)
            return Exception_Type::EXCEPTION_BITWISE_NON_INT;

        const int64_t num = stack[sp-2].as_int();
        const int64_t shift_amt = stack[sp-1].as_int();
        const int64_t shifted_res = (num >> shift_amt) & ((1LL << (48LL - shift_amt)) - 1);
        stack[sp-2].box_int(shifted_res);

        sp--;
        break;
    }

    case Inst_Type::INST_SAR:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] >>= stack[sp-1];
        sp--;
        break;

    case INST_AND:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] &= stack[sp-1];
        sp--;
        break;

    case INST_OR:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] |= stack[sp-1];
        sp--;
        break;

    case INST_XOR:
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-2] ^= stack[sp-1];
        sp--;
        break;

    case INST_NOT:
        if (sp < 1)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        stack[sp-1] = ~stack[sp-1];
        break;

    case INST_READ: {
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        // check the 'read size'
        const int64_t read_size = inst.operand.as_int();
        if ((read_size != 8) && (read_size != 16) && (read_size != 32) && (read_size != 64))
            return Exception_Type::EXCEPTION_INVALID_READ_WRITE_SIZE;

        // check ptr type
        if (stack[sp-1].get_type() != Nan_Type::PTR && stack[sp-1].get_type() != Nan_Type::INT)
            return Exception_Type::EXCEPTION_INVALID_MEM_ADDR;

        // check 'ptr' value
        if (stack[sp-1].get_type() == Nan_Type::INT && stack[sp-1].as_int() < 0)
            return Exception_Type::EXCEPTION_INVALID_MEM_ADDR;

        // check 'ptr' value
        if ((uint64_t)stack[sp-1].as_ptr() >= program.memory.size())
            return Exception_Type::EXCEPTION_INVALID_MEM_ADDR;

        // get a byte from a double in the static memory (not sure if this will ever be used)
        if (program.memory[static_cast<size_t>(stack[sp-1].as_int())].get_type() == Nan_Type::DOUBLE) {
            double value = program.memory[static_cast<size_t>(stack[sp-1].as_int())].as_double();
            const uint64_t *const new_value = (uint64_t*)(&value);
            stack[sp-1] = CAST_TO_SIZE(read_size, *new_value);
        } else {
            stack[sp-1] = CAST_TO_SIZE(read_size, program.memory[static_cast<size_t>(stack[sp-1].as_int())].as_int());
        }
        break;
    }

    case INST_WRITE: {
        if (sp < 2)
            return Exception_Type::EXCEPTION_STACK_UNDERFLOW;

        // check the 'write size'
        const int64_t write_size = inst.operand.as_int();
        if ((write_size != 8) && (write_size != 16) && (write_size != 32) && (write_size != 64))
            return Exception_Type::EXCEPTION_INVALID_READ_WRITE_SIZE;

        // check ptr type
        if (stack[sp-2].get_type() != Nan_Type::PTR && stack[sp-2].get_type() != Nan_Type::INT)
            return Exception_Type::EXCEPTION_INVALID_MEM_ADDR;

        // check 'ptr' value
        if (stack[sp-2].get_type() == Nan_Type::INT && stack[sp-2].as_int() < 0)
            return Exception_Type::EXCEPTION_INVALID_MEM_ADDR;

        // check ptr value
        if ((uint64_t)stack[sp-2].as_ptr() >= program.memory.size())
            return Exception_Type::EXCEPTION_INVALID_MEM_ADDR;

        // get a byte from a double in the stack (not shure if this will ever be used)
        if (stack[sp-1].get_type() == Nan_Type::DOUBLE) {
            const double value = stack[sp-1].as_double();
            const uint64_t *const new_value = (uint64_t*)(&value);
            program.memory[static_cast<size_t>(stack[sp-2].as_int())] = CAST_TO_SIZE(write_size, *new_value);
        } else {
            program.memory[static_cast<size_t>(stack[sp-2].as_int())] = CAST_TO_SIZE(write_size, stack[sp-1].as_int());
        }
        sp -= 2;
        break;
    }

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

void Vm::dump_memory() {
    // print the memory
    std::cout << "Vm Memory (" << program.memory.size() << " element" << (program.memory.size() != 1 ? "s" : "") << "):" << std::endl;
    for (size_t i = 0; i < program.memory.size(); i++) {
        switch (program.memory[i].get_type()) {
        case Nan_Type::DOUBLE:
            std::cout << "    # Double: " << program.memory[i].as_double() << std::endl;
            break;

        case Nan_Type::INT:
            std::cout << "    # Int: " << program.memory[i].as_int() << std::endl;
            break;

        case Nan_Type::PTR:
            std::cout << "    # Ptr: " << program.memory[i].as_ptr() << std::endl;
            break;

        case Nan_Type::EXCEPTION:
        default:
            std::cerr << "ERROR: Unknown variable data type in the memory." << std::endl;
            exit(1);
        }
    }

    if (program.memory.size() == 0)
        std::cout << "    [Empty]" << std::endl;
}

// native functions
// WARNING: we assume that the state of the machine was checked before calling this native functions

void Vm::native_malloc() {
    // get the size of the memory to allocate
    const size_t size = static_cast<size_t>(stack[sp-1].as_int());

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

void Vm::native_fwrite() {
    // get the string size
    const size_t str_size = static_cast<size_t>(stack[sp-2].as_int());

    // get the string (convert from Nan_Box to char)
    std::vector<char> buf;
    buf.reserve(str_size);
    for (size_t i = 0; i < str_size; i++) {
        buf.push_back(static_cast<char>(program.memory[static_cast<size_t>(stack[sp-1].as_int())+i].as_int()));
    }

    // get the file 'pointer'
    FILE *file_ptr;
    switch (stack[sp-3].as_int()) {
    case 0:
        file_ptr = stdin;
        break;

    case 1:
        file_ptr = stdout;
        break;

    case 3:
        file_ptr = stderr;
        break;

    default:
        std::cerr << "ERROR: Invalid file descriptor." << std::endl;
        exit(1);
    }

    // make the native function call
    fwrite(buf.data(), 1, str_size, file_ptr);
}
