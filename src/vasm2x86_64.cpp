#include <iostream>
#include <vector>

#include "vm.h"
#include "program_args.h"

void program_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [args]" << std::endl;
    std::cerr << "    args: -i: Input file name to run." << std::endl;
}

int main(int argc, char const *argv[]) {
    const std::vector<std::string_view> args(argv, argv + argc);

    if (!program_args::has_option(args, "-i")) {
        std::cerr << "ERROR: Expected argument '-i'." << std::endl;
        program_usage(args.at(0).data());
        exit(1);
    }

    const std::string_view input_file_path = program_args::get_option(args, "-i");
    if (input_file_path == "") {
        std::cerr << "ERROR: Option '-i' requires a parameter." << std::endl;
        program_usage(args.at(0).data());
        exit(1);
    }

    Program p;
    p.read_from_file(input_file_path.data());

    std::cout << "%define SYS_EXIT 60" << std::endl;
    std::cout << "%define STACK_CAP " << STACK_CAP << std::endl << std::endl;
    std::cout << "section .text" << std::endl;
    std::cout << "    global _start" << std::endl << std::endl;
    std::cout << "_start:" << std::endl;

    for (Inst& inst: p.insts) {
        switch (inst.type) {
        case Inst_Type::INST_NOP:
            break;
        case Inst_Type::INST_EXIT:
            break;
        case Inst_Type::INST_PUSH:
            break;
        case Inst_Type::INST_POP:
            break;
        case Inst_Type::INST_SWAP:
            break;
        case Inst_Type::INST_DUP:
            break;
        case Inst_Type::INST_PRINT:
            break;
        case Inst_Type::INST_TD:
            break;
        case Inst_Type::INST_TI:
            break;
        case Inst_Type::INST_TP:
            break;
        case Inst_Type::INST_ADD:
            break;
        case Inst_Type::INST_SUB:
            break;
        case Inst_Type::INST_MUL:
            break;
        case Inst_Type::INST_DIV:
            break;
        case Inst_Type::INST_MOD:
            break;
        case Inst_Type::INST_AND:
            break;
        case Inst_Type::INST_OR:
            break;
        case Inst_Type::INST_XOR:
            break;
        case Inst_Type::INST_NOT:
            break;
        case Inst_Type::INST_NATIVE:
            break;
        case Inst_Type::INST_SHL:
            break;
        case Inst_Type::INST_SHR:
            break;
        case Inst_Type::INST_SAR:
            break;
        case Inst_Type::INST_JMP:
            break;
        case Inst_Type::INST_EQU:
            break;
        case Inst_Type::INST_JMP_IF:
            break;
        case Inst_Type::INST_CALL:
            break;
        case Inst_Type::INST_RET:
            break;
        case Inst_Type::INST_READ:
            break;
        case Inst_Type::INST_WRITE:
            break;
        case Inst_Type::INST_DUMP_STACK:
            break;
        case Inst_Type::INST_DUMP_MEMORY:
            break;
        case Inst_Type::INST_HALT:
            break;
        case Inst_Type::INST_COUNT:
            break;
        default:
            break;
        }
    }

    std::cout << "    mov rax, SYS_EXIT" << std::endl;
    std::cout << "    mov rdi, 0" << std::endl;
    std::cout << "    syscall" << std::endl << std::endl;
    std::cout << "sector .data" << std::endl;
    std::cout << "stack_top: stack" << std::endl << std::endl;
    std::cout << "sector .bss" << std::endl;
    std::cout << "stack: resq STACK_CAP" << std::endl;

    return 0;
}
