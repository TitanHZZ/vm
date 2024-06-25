#include <iostream>
#include <cstring>

#include "program.h"
#include "program_args.h"

void program_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [args]" << std::endl;
    std::cerr << "    args: -i: Input file name with the source code." << std::endl;
    std::cerr << "          -o: Output file name to save the executable." << std::endl;
}

int main(int argc, char* argv[]) {
    const std::vector<std::string_view> args(argv, argv + argc);

    if (!program_args::has_option(args, "-i") || !program_args::has_option(args, "-o")) {
        std::cerr << "ERROR: Expected arguments '-i', '-o'." << std::endl;
        program_usage(args.at(0).data());
        exit(1);
    }

    const std::string_view input_file_path = program_args::get_option(args, "-i");
    if (input_file_path == "") {
        std::cerr << "ERROR: Option '-i' requires a parameter." << std::endl;
        program_usage(args.at(0).data());
        exit(1);
    }

    const std::string_view output_file_path = program_args::get_option(args, "-o");
    if (output_file_path == "") {
        std::cerr << "ERROR: Option '-o' requires a parameter." << std::endl;
        program_usage(args.at(0).data());
        exit(1);
    }

    // Program p;
    // p.parse_from_file(input_file_path.data());
    // p.write_to_file(output_file_path.data());
    return 0;
}
