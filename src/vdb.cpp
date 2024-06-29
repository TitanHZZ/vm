#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>

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

class Vdb {
public:
    Vdb(Vm &vm, Program &p) : vm(vm), p(p) {}

    void debug() {
        vm.execute_program(true);

        std::string user_option;
        Vdb_Cmd_Parser cmd_parser;
        std::cout << "vdb: a gdb style debugger for vm!\nLoading program..." << std::endl;
        while (true) {
            std::cout << "(vdb) ";

            // check for I/O error or stdin closing
            if (!std::getline(std::cin, user_option))
                break;

            Vdb_Command cmd = cmd_parser.get_vdb_command(user_option);
            switch (cmd.type) {
            case Vdb_Command_Type::RUN:
                while(vm.next() != Exception_Type::EXCEPTION_EXIT && !breakpoints.contains(vm.get_ip()));
                break;

            case Vdb_Command_Type::DISAS:
                disassemble();
                break;

            case Vdb_Command_Type::NI:
                vm.next();
                break;

            case Vdb_Command_Type::BREAK:
                set_breakpoint(cmd);
                break;

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
    }

private:
    void get_label_def() {
        size_t label_suffix = 0;
        for (const Inst& inst: p.insts) {
            if (inst_operand_might_be_label(inst.type)) {
                // jmp_addr_label_names.emplace(inst.operand.as_ptr(), "label_" + std::to_string(label_suffix));
                labels["label_" + std::to_string(label_suffix)] = (uint64_t)inst.operand.as_ptr();
                label_suffix++;
            }
        }
    }

    void disassemble() {
        // std::unordered_map<void*, std::string> jmp_addr_label_names;

        size_t inst_count = 0;
        for (const Inst& inst: p.insts) {
            // print labels

            if (jmp_addr_label_names.contains((void*)inst_count))
                std::cout << std::endl << "           " << jmp_addr_label_names.at((void*)inst_count) << ":" << std::endl;

            // print instruction addr
            std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << inst_count;
            std::cout << std::dec << std::setfill(' '); // reset the console to the defaults

            // print breakpoint indicator if needed
            std::cout << (breakpoints.contains(inst_count) ? " *" : "  ");

            // print instruction with "current instruction" indicator
            std::cout << (vm.get_ip() == inst_count ? " ->  " : "     ") << inst_type_as_cstr(inst.type);

            // print instruction operand
            if (inst_requires_operand(inst.type)) {
                if (inst_operand_might_be_label(inst.type)) {
                    std::cout << " " << jmp_addr_label_names.at(inst.operand.as_ptr());
                } else if (inst_operand_might_be_function(inst.type)) {
                    std::cout << " " << Vm::native_funcs_names[inst.operand.as_int()];
                } else {
                    switch (inst.operand.get_type()) {
                    case Nan_Type::DOUBLE:
                        // prints in scientific notation
                        std::cout << " " << inst.operand.as_double();
                        break;

                    case Nan_Type::INT:
                        std::cout << " " << inst.operand.as_int();
                        break;

                    case Nan_Type::PTR:
                        std::cout << " " << inst.operand.as_ptr();
                        break;

                    case Nan_Type::EXCEPTION:
                    default:
                        std::cerr << "ERROR: Unknown variable data type in the stack." << std::endl;
                        exit(1);
                    }
                }
            }

            std::cout << std::endl;
            inst_count++;
        }
    }

    void set_breakpoint(Vdb_Command &cmd) {
        if (cmd.args[0].type == Vdb_Token_Type::NUMBER) {
            // parse the number
            size_t addr;
            std::istringstream iss(cmd.args[0].value);
            iss >> std::hex >> addr;

            // set breakpoint
            if (addr < p.insts.size()) {
                breakpoints.insert(addr);
            } else {
                std::cout << "Failed to set break point. Address is bigger than the number of instructions!" << std::endl;
            }
        } else {
            // check if label exists
            if (!labels.contains(cmd.args[0].value)) {
                std::cout << "Failed to set break point. Label `" << cmd.args[0].value << "` does not  exist!" << std::endl;
            } else {
                breakpoints.insert(labels[cmd.args[0].value]);
            }
        }
    }

    Vm &vm;
    Program &p;

    std::unordered_set<uint64_t> breakpoints;
    std::unordered_map<std::string, uint64_t> labels;
};

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
    Vdb d(vm, p);
    d.debug();

    return 0;
}
