#include <iostream>
#include <algorithm>

#include "vdb_command_parser.h"
#include "program_args.h"
#include "vm.h"
#include "parser.h"

// simple command line language:
// run                  -> run the program until a breakpoint or the end
// ni                   -> run just the next instruction
// disas / disassemble  -> disassemble the byte code
// break 0              -> set breakpoint at addr 0
// info (break | ...)   -> get information about something
// delete (break 0 | ...) -> delete something
// x 0 10               -> inspect (print) the memory from addr 0 to 10

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

    std::unordered_set<size_t> breakpoints;

    std::string user_option;
    Vdb_Cmd_Parser cmd_parser;
    std::cout << "vdb: a gdb style debugger for vm!" << std::endl;
    while (true) {
        std::cout << "(vdb) ";

        // check for I/O error or stdin closing
        if (!std::getline(std::cin, user_option))
            break;

        Vdb_Command cmd = cmd_parser.get_vdb_command(user_option);
        switch (cmd.type) {
        case Vdb_Command_Type::RUN:
        case Vdb_Command_Type::NI:
        case Vdb_Command_Type::DISAS:
        case Vdb_Command_Type::BREAK:
        case Vdb_Command_Type::INFO:
        case Vdb_Command_Type::DELETE:
        case Vdb_Command_Type::X:
            std::cout << "Got command!" << std::endl;
            break;

        case Vdb_Command_Type::NOTHING:
        case Vdb_Command_Type::UNKNOWN:
        default:
            break;
        }
    }

    std::cout << std::endl;
    return 0;
}
