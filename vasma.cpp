#include <iostream>
#include <cstring>

#include "program.h"

void next_arg(int *argc_p, char ***argv_p) {
    (*argc_p)--;
    (*argv_p)++;
}

void print_usage(const char *program_name) {
    std::cerr << "Usage: " << program_name << " [args]" << std::endl;
    std::cerr << "    args: -i: Input file name with the source code." << std::endl;
    std::cerr << "          -o: Output file name to save the executable." << std::endl;
}

int main(int argc, char** argv) {
    // save program name
    const char *program_name = argv[0];

    // ignore program name in args
    next_arg(&argc, &argv);

    if (argc != 4) {
        std::cerr << "ERROR: Expected 4 arguments but got " << argc << "." << std::endl;
        print_usage(program_name);
        exit(1);
    }

    const char *arg1 = argv[0];
    const char *arg2 = argv[2];

    char *input_file_name = nullptr;
    char *output_file_name = nullptr;

    if (strcmp(arg1, "-i") == 0) {
        if (strcmp(arg2, "-o") == 0) {
            input_file_name = argv[1];
            output_file_name = argv[3];
        }
    } else if (strcmp(arg2, "-i") == 0) {
        if (strcmp(arg1, "-o") == 0) {
            input_file_name = argv[3];
            output_file_name = argv[1];
        }
    }

    if (input_file_name == nullptr || output_file_name == nullptr) {
        std::cerr << "ERROR: An unknown error occurred when parsing the arguments." << std::endl;
        print_usage(program_name);
        exit(1);
    }

    Program p;
    p.parse_from_file(input_file_name);
    p.write_to_file(output_file_name);
    return 0;
}
