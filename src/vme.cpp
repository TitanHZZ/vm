#include <iostream>
#include <cstring>

#include "vm.h"
#include "program_args.h"

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

void program_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [args]" << std::endl;
    std::cerr << "    args: -i: Input file name to run." << std::endl;
}

int main(int argc, char* argv[]) {
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

    Vm vm;
    vm.execute_program(p);

    return 0;
}
