#include "program.h"
#include "vm.h"

Program::Program(const Inst *list, const size_t count) {
    for (size_t i = 0; i < count; i++) {
        insts.push_back(list[i]);
    }
}

void Program::write_to_file(const char *path, const Inst *program, const size_t program_size) {
    std::ofstream file;
    file.open(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR: An error occured when trying to write a program to '" << path << "'" << std::endl;
        exit(1);
    }

    file.write((const char *)program, static_cast<std::streamsize>(sizeof(Inst) * program_size));
    file.close();
}

void Program::write_to_file(const char *path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR: An error occured when trying to write a program to '" << path << "'" << std::endl;
        exit(1);
    }

    file << *this;
    file.close();
}

void Program::read_from_file(const char *path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR: An error occured when trying to read a program from '" << path << "'" << std::endl;
        exit(1);
    }

    file >> *this;
    file.close();
}

void Program::print_program(bool with_labels) {
    size_t label_suffix = 0;
    std::unordered_map<void*, std::string> jmp_addr_label_names;

    // get places for labels if requested
    if (with_labels) {
        for (Inst& inst: insts) {
            if (inst_operand_might_be_label(inst.type) && jmp_addr_label_names.contains(inst.operand.as_ptr()) == false) {
                jmp_addr_label_names.emplace(inst.operand.as_ptr(), "label_" + std::to_string(label_suffix));
                label_suffix++;
            }
        }
    }

    size_t inst_count = 0;
    for (Inst& inst: insts) {
        // print labels
        if (with_labels && jmp_addr_label_names.contains((void*)inst_count))
            std::cout << std::endl << jmp_addr_label_names.at((void*)inst_count) << ":" << std::endl;

        // print instruction and its operand
        std::cout << "    " << inst_type_as_cstr(inst.type);
        if (inst_requires_operand(inst.type)) {
            if (with_labels && inst_operand_might_be_label(inst.type)) {
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
