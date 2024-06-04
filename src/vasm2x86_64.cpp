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

    // get places for labels
    size_t label_suffix = 0;
    std::unordered_map<void*, std::string> jmp_addr_label_names;
    for (Inst& inst: p.insts) {
        if (inst_operand_might_be_label(inst.type) && jmp_addr_label_names.contains(inst.operand.as_ptr()) == false) {
            jmp_addr_label_names.emplace(inst.operand.as_ptr(), "label_" + std::to_string(label_suffix));
            label_suffix++;
        }
    }

    std::cout << "BITS 64" << std::endl << std::endl;
    std::cout << "%define SYS_EXIT 60" << std::endl;
    std::cout << "%define STACK_CAP " << STACK_CAP << std::endl << std::endl;
    std::cout << "extern Nan_Box_box_int, Nan_Box_add, Nan_Box_print, Nan_Box_equ, Nan_Box_get_value" << std::endl << std::endl;
    std::cout << "section .text" << std::endl;
    std::cout << "    global _start" << std::endl << std::endl;
    std::cout << "_start:" << std::endl;

    size_t inst_count = 0;
    for (Inst& inst: p.insts) {
        // print labels
        if (jmp_addr_label_names.contains((void*)inst_count))
            std::cout << jmp_addr_label_names.at((void*)inst_count) << ":" << std::endl;

        switch (inst.type) {
        case Inst_Type::INST_NOP:
            break;
        case Inst_Type::INST_EXIT:
            break;
        case Inst_Type::INST_PUSH:
            std::cout << "    ; push " << inst.operand << std::endl;
            std::cout << "    mov rdi, [stack_top]" << std::endl;
            std::cout << "    mov rsi, " << inst.operand << std::endl;
            std::cout << "    call Nan_Box_box_int" << std::endl;
            std::cout << "    add QWORD [stack_top], " << WORD_SIZE << std::endl << std::endl;
            break;
        case Inst_Type::INST_POP:
            break;
        case Inst_Type::INST_SWAP:
            std::cout << "    ; swap " << inst.operand << std::endl;
            std::cout << "    mov rax, [stack_top]" << std::endl;
            std::cout << "    mov rbx, [rax-8]" << std::endl;
            std::cout << "    xchg rbx, [rax-" << WORD_SIZE * (static_cast<uint64_t>(inst.operand.as_int()) + 1) << "]" << std::endl;
            std::cout << "    xchg rbx, [rax-8]" << std::endl << std::endl;
            break;
        case Inst_Type::INST_DUP:
            std::cout << "    ; dup " << inst.operand << std::endl;
            std::cout << "    mov rax, [stack_top]" << std::endl;
            std::cout << "    mov rbx, [rax-" << WORD_SIZE * (static_cast<uint64_t>(inst.operand.as_int()) + 1) << "]" << std::endl;
            std::cout << "    mov [rax], rbx" << std::endl;
            std::cout << "    add QWORD [stack_top], 8" << std::endl << std::endl;
            break;
        case Inst_Type::INST_PRINT:
            std::cout << "    ; print " << inst.operand << std::endl;
            std::cout << "    mov rdi, [stack_top]" << std::endl;
            std::cout << "    sub rdi, " << WORD_SIZE * (static_cast<uint64_t>(inst.operand.as_int()) + 1) << std::endl;
            std::cout << "    call Nan_Box_print" << std::endl << std::endl;
            break;
        case Inst_Type::INST_TD:
            break;
        case Inst_Type::INST_TI:
            break;
        case Inst_Type::INST_TP:
            break;
        case Inst_Type::INST_ADD:
            std::cout << "    ; add" << std::endl;
            std::cout << "    mov rax, [stack_top]" << std::endl;
            std::cout << "    lea rdi, [rax-" << WORD_SIZE * 2 << "]" << std::endl;
            std::cout << "    lea rsi, [rax-" << WORD_SIZE << "]" << std::endl;
            std::cout << "    call Nan_Box_add" << std::endl;
            std::cout << "    sub QWORD [stack_top], 8" << std::endl << std::endl;
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
            std::cout << "    ; equ" << std::endl;
            std::cout << "    mov rax, [stack_top]" << std::endl;
            std::cout << "    lea rdi, [rax-16]" << std::endl;
            std::cout << "    lea rsi, [rax-8]" << std::endl;
            std::cout << "    call Nan_Box_equ" << std::endl;
            std::cout << "    mov rdi, [stack_top]" << std::endl;
            std::cout << "    sub rdi, 16" << std::endl;
            std::cout << "    mov rsi, rax" << std::endl;
            std::cout << "    call Nan_Box_box_int" << std::endl;
            std::cout << "    sub QWORD [stack_top], 8" << std::endl << std::endl;
            break;
        case Inst_Type::INST_JMP_IF:
            std::cout << "    ; jif " << jmp_addr_label_names.at(inst.operand.as_ptr()) << std::endl;
            std::cout << "    mov rdi, [stack_top]" << std::endl;
            std::cout << "    sub rdi, 8" << std::endl;
            std::cout << "    call Nan_Box_get_value" << std::endl;
            std::cout << "    sub QWORD [stack_top], 8" << std::endl;
            std::cout << "    cmp rax, 0" << std::endl;
            std::cout << "    jne label_0" << std::endl << std::endl;
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

        inst_count++;
    }

    std::cout << "    ; exit" << std::endl;
    std::cout << "    mov rax, SYS_EXIT" << std::endl;
    std::cout << "    mov rdi, 0" << std::endl;
    std::cout << "    syscall" << std::endl << std::endl;
    std::cout << "section .data" << std::endl;
    std::cout << "stack_top: dq stack" << std::endl << std::endl;
    std::cout << "section .bss" << std::endl;
    std::cout << "stack: resq STACK_CAP" << std::endl;

    return 0;
}
