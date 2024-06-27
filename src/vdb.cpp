#include <iostream>
#include <algorithm>

#include "program_args.h"
#include "program.h"
#include "parser.h"
#include "vm.h"

typedef enum {
    OPTION_RUN = 0,
    OPTION_BREAKPOINT,
    OPTION_NEXT_INSTRUCTION,
    OPTION_EXIT,
    OPTION_HELP,
    OPTION_NOTHING,
    OPTION_UNKNOWN
} Option_Type;

constexpr static const char *const strs_for_option[][2] = {
    {"r", "run"},               // OPTION_RUN
    {"break", "br"},            // OPTION_BREAKPOINT
    {"next instruction", "ni"}, // OPTION_NEXT_INSTRUCTION
    {"exit", "e"},              // OPTION_EXIT
    {"?", "help"},              // OPTION_HELP
    {"\0", "\0"},               // OPTION_NOTHING
    {"\0", "\0"}                // OPTION_UNKNOWN
};

Option_Type str_as_option(std::string &str) {
    // replace tabs with spaces
    std::replace(std::begin(str), std::end(str), '\t', ' ');

    // convert the string to lower case
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });

    // remove consecutive spaces
    std::string::iterator iter = std::unique(str.begin(), str.end(), [](char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); });
    str.erase(iter, str.end());

    // check if we have an empty string
    if (str == "" || str == " ")
        return Option_Type::OPTION_NOTHING;

    for (size_t i = 0; i < sizeof(strs_for_option) / sizeof(strs_for_option[0]); i++) {
        for (size_t j = 0; j < sizeof(strs_for_option[0]) / sizeof(strs_for_option[0][0]); j++) {
            if (str == strs_for_option[i][j]) {
                // we got our option
                return (Option_Type) i;
            }
        }
    }

    return Option_Type::OPTION_UNKNOWN;
}

void program_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [args]" << std::endl;
    std::cerr << "    args: -i: Input file name with the source code." << std::endl;
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
    Lexer lexer(input_file_path.data());
    Parser parser(lexer.tokenize());
    parser.parse(&p);

    Vm vm(p);
    vm.execute_program(true);

    bool needs_exit = false;
    std::string user_option;
    std::cout << "vdb: a gdb style debugger for vm!" << std::endl;
    while (!needs_exit) {
        std::cout << "(vdb) ";

        // check for I/O error or stdin closing
        if (!std::getline(std::cin, user_option))
            needs_exit = true;

        switch (str_as_option(user_option)) {
        case Option_Type::OPTION_RUN: // TODO: this needs to respect the breakpoints
            while(vm.next() != Exception_Type::EXCEPTION_EXIT);
            break;

        case Option_Type::OPTION_HELP:
            std::cout << "Help menu is not yet complete!" << std::endl;
            break;

        case Option_Type::OPTION_EXIT:
            return 0;

        case Option_Type::OPTION_NOTHING:
            break;

        case Option_Type::OPTION_BREAKPOINT:
        case Option_Type::OPTION_NEXT_INSTRUCTION:
        case Option_Type::OPTION_UNKNOWN:
        default:
            std::cout << "Unknown option `" << user_option << "`. For possible commands, please use `?` or `help`" << std::endl;
            break;
        }
    }

    std::cout << std::endl;
    return 0;
}
