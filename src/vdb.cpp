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
        bool program_finished = false;
        std::cout << "vdb: a gdb style debugger for vm!\nLoading program..." << std::endl;
        get_label_def();
        while (true) {
            std::cout << "(vdb) ";

            // check for I/O error or stdin closing
            if (!std::getline(std::cin, user_option))
                break;

            Vdb_Command cmd = cmd_parser.get_vdb_command(user_option);
            switch (cmd.type) {
            case Vdb_Command_Type::RUN:
                if (program_finished) {
                    std::cout << "Program has finished. Cannot continue." << std::endl;
                    break;
                }

                while (!program_finished) {
                    const Exception_Type ret = vm.next();

                    if (ret == Exception_Type::EXCEPTION_EXIT) {
                        std::cout << "Program finished." << std::endl;
                        program_finished = true;
                    }

                    if (breakpoints.contains(vm.get_ip()))
                        break;
                }
                break;

            case Vdb_Command_Type::DISAS:
                disassemble();
                break;

            case Vdb_Command_Type::NI:
                if (program_finished) {
                    std::cout << "Program has finished. Cannot continue." << std::endl;
                    break;
                }

                if (vm.next() == EXCEPTION_EXIT) {
                    std::cout << "Program finished." << std::endl;
                    program_finished = true;
                }
                break;

            case Vdb_Command_Type::BREAK:
                set_breakpoint(cmd);
                break;

            case Vdb_Command_Type::HELP:
                std::cout << "Available commands:" << std::endl;
                std::cout << "    run                   -> run the program until a breakpoint or the end" << std::endl;
                std::cout << "    ni                    -> run just the next instruction" << std::endl;
                std::cout << "    disas                 -> disassemble the byte code" << std::endl;
                std::cout << "    break (label | 0)     -> set breakpoint at label or addr 0" << std::endl;
                std::cout << "    info (break | ...)    -> get information about some command" << std::endl;
                std::cout << "    delete 0              -> delete breakpoint previously set at addr 0" << std::endl;
                std::cout << "    x 0 10                -> inspect (print) the memory from addr 0 to 10" << std::endl;
                break;

            case Vdb_Command_Type::INFO:
                switch (Vdb_Cmd_Parser::str_as_vdb_cmd(cmd.args[0].value)) {
                case Vdb_Command_Type::BREAK:
                    // print breakpoints
                    if (breakpoints.size() == 0) {
                        std::cout << "No breakpoints set." << std::endl;
                        break;
                    }

                    std::cout << "Breakpoints: ";
                    for (const auto& elem: breakpoints)
                        std::cout << elem << " ";
                    std::cout << std::endl;
                    break;

                case Vdb_Command_Type::DELETE:
                case Vdb_Command_Type::DISAS:
                case Vdb_Command_Type::HELP:
                case Vdb_Command_Type::INFO:
                case Vdb_Command_Type::NI:
                case Vdb_Command_Type::NOTHING:
                case Vdb_Command_Type::RUN:
                case Vdb_Command_Type::X:
                case Vdb_Command_Type::UNKNOWN:
                default:
                    std::cout << "`" << cmd.args[0].value << "` is not a valid subcommand for `info`." << std::endl;
                    break;
                }
                break;

            case Vdb_Command_Type::DELETE: {
                uint64_t addr;
                std::istringstream iss(cmd.args[0].value);
                iss >> (cmd.args[0].value.find('x') == std::string::npos ? std::dec : std::hex) >> addr;

                if (!breakpoints.contains(addr)) {
                    std::cout << "No such breakpoint with addr `" << addr << "`." << std::endl;
                    break;
                }

                breakpoints.erase(addr);
            }
            break;

            case Vdb_Command_Type::X: {
                uint64_t base_addr, top_addr;

                std::istringstream iss0(cmd.args[0].value);
                iss0 >> (cmd.args[0].value.find('x') == std::string::npos ? std::dec : std::hex) >> base_addr;

                std::istringstream iss1(cmd.args[1].value);
                iss1 >> (cmd.args[1].value.find('x') == std::string::npos ? std::dec : std::hex) >> top_addr;

                if (base_addr > top_addr) {
                    std::cout << "Base addr cannot be bigger than the top addr." << std::endl;
                    break;
                }

                if (base_addr >= p.memory.size() || top_addr >= p.memory.size()) {
                    std::cout << "Addrs are out of range. Current memory size: " << p.memory.size() << std::endl;
                    break;
                }

                for (uint64_t i = base_addr; i <= top_addr; i++) {
                    switch (p.memory[i].get_type()) {
                    case Nan_Type::DOUBLE:
                        std::cout << "    # Double: " << p.memory[i].as_double() << std::endl;
                        break;

                    case Nan_Type::INT:
                        std::cout << "    # Int: " << p.memory[i].as_int() << std::endl;
                        break;

                    case Nan_Type::PTR:
                        std::cout << "    # Ptr: " << p.memory[i].as_ptr() << std::endl;
                        break;

                    case Nan_Type::EXCEPTION:
                    default:
                        std::cerr << "ERROR: Unknown variable data type in the memory." << std::endl;
                        exit(1);
                    }
                }
            }
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
                label_to_addr["label_" + std::to_string(label_suffix)] = (uint64_t)inst.operand.as_ptr();
                addr_to_label[(uint64_t)inst.operand.as_ptr()] = "label_" + std::to_string(label_suffix);
                label_suffix++;
            }
        }
    }

    void disassemble() {
        size_t inst_count = 0;
        for (const Inst& inst: p.insts) {
            // print labels

            if (addr_to_label.contains(inst_count))
                std::cout << std::endl << "           " << addr_to_label.at(inst_count) << ":" << std::endl;

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
                    std::cout << " " << addr_to_label.at((uint64_t)inst.operand.as_ptr());
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
        uint64_t addr;

        if (cmd.args[0].type == Vdb_Token_Type::NUMBER) {
            std::istringstream iss(cmd.args[0].value);
            iss >> (cmd.args[0].value.find('x') == std::string::npos ? std::dec : std::hex) >> addr;

            // set breakpoint
            if (addr >= p.insts.size()) {
                std::cout << "Failed to set break point. Address is bigger than the number of instructions!" << std::endl;
                return;
            }
        } else {
            // check if label exists
            if (!label_to_addr.contains(cmd.args[0].value)) {
                std::cout << "Failed to set break point. Label `" << cmd.args[0].value << "` does not  exist!" << std::endl;
                return;
            } else {
                addr = label_to_addr[cmd.args[0].value];
            }
        }

        if (breakpoints.contains(addr))
            std::cout << "Failed to set break point. Break point at addr `" << addr << "` already set." << std::endl;
        else
            breakpoints.insert(addr);
    }

    Vm &vm;
    Program &p;

    std::unordered_set<uint64_t> breakpoints;
    std::unordered_map<std::string, uint64_t> label_to_addr;
    std::unordered_map<uint64_t, std::string> addr_to_label;
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
