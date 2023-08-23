#include <iostream>
#include <cstring>

#include "vm.h"

/*// count up to 50
const Inst program2[] = {
    MAKE_INST_PUSH(0),
    MAKE_INST_PUSH(1),
    MAKE_INST_ADD(),
    MAKE_INST_DUP(0),
    MAKE_INST_PUSH(50),
    MAKE_INST_EQU(),
    MAKE_INST_JMP_IF(8),
    MAKE_INST_JMP(1),
    MAKE_INST_POP(),
    MAKE_INST_EXIT()
};
// generates fibonacci sequence
const Inst program[] = {
    MAKE_INST_PUSH(0),
    MAKE_INST_PUSH(1),
    MAKE_INST_DUP(1),
    MAKE_INST_DUP(1),
    MAKE_INST_ADD(),
    MAKE_INST_JMP(2),
};
constexpr size_t program_size = sizeof(program) / sizeof(program[0]);*/
// Vm vm;
// Program p;
// Program p(program, program_size);
// Program p(std::move(program), program_size); // takes ownership of the program

void next_arg(int *argc_p, char ***argv_p) {
    (*argc_p)--;
    (*argv_p)++;
}

void print_usage(const char *program_name) {
    std::cerr << "Usage: " << program_name << " [args]" << std::endl;
    std::cerr << "    args: -i: Input file name to run." << std::endl;
}

int main(int argc, char** argv) {
    // save program name
    const char *program_name = argv[0];

    // ignore program name in args
    next_arg(&argc, &argv);

    if (argc != 2) {
        std::cerr << "ERROR: Expected 2 arguments but got " << argc << "." << std::endl;
        print_usage(program_name);
        exit(1);
    }

    const char *arg1 = argv[0];

    char *input_file_name = nullptr;
    if (strcmp(arg1, "-i") == 0) {
        input_file_name = argv[1];
    }

    if (input_file_name == nullptr) {
        std::cerr << "ERROR: An unknown error occurred when parsing the arguments." << std::endl;
        print_usage(program_name);
        exit(1);
    }

    Program p;
    p.read_from_file(input_file_name);

    Vm vm;
    vm.execute_program(p);

    return 0;
}
