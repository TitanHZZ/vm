#include <iostream>

#include "program.h"
#include "program_args.h"
#include "parser.h"

void program_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [args]" << std::endl;
    std::cerr << "    args: -i: Input file name of the executable to disassemble." << std::endl;
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

    const bool print_with_labels = program_args::has_option(args, "-l");

    Program p;
    Lexer lexer(input_file_path.data());
    Parser parser(lexer.tokenize());
    parser.parse(&p);
    p.print_program(print_with_labels);
    return 0;
}
