#include "program.h"
#include "vm.h"

Program::Program(const Inst *list, const size_t count) {
    for (size_t i = 0; i < count; i++) {
        insts.push_back(list[i]);
    }
}
/*Program::Program(Inst (&&inst_list)[], const size_t count) {
    for (size_t i = 0; i < count; i++) {
        insts.push_back(std::move(inst_list[i]));
    }
}*/

void Program::write_to_file(const char *path, const Inst *program, const size_t program_size) {
    std::ofstream file;
    file.open(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR: An error occured when trying to write a program to '" << path << "'" << std::endl;
        exit(1);
    }

    file.write((const char *)program, sizeof(Inst) * program_size);
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

void Program::string_trim(std::string& str) {
    // trim string
    while (str.size() > 0 && str.at(0) == ' ') str.erase(str.begin());      // trim left
    while (str.size() > 0 && str.at(str.size() - 1) == ' ') str.pop_back(); // trim right
}
 
void Program::string_split_by_delimeter(std::string& line, const char ch, const bool get_first_substring) {
    if (line.size() < 1)
        return;

    // check for the 'ch' in the line
    const size_t delimeter_index = line.find_first_of(ch);
    if (delimeter_index == std::string::npos)
        return;

    // split line by delimeter
    if (get_first_substring) {
        line = line.substr(0, delimeter_index);
    } else {
        line = line.substr(delimeter_index + 1, line.size() - delimeter_index - 1);
    }
}

void Program::get_label_definitions(std::ifstream& file, std::unordered_map<std::string, Label>& labels) {
    size_t line_count = 0;
    size_t inst_count = 0;
    std::string line;

    while (std::getline(file, line)) {
        line_count++;

        // ignore comments in same line as instruction and ignore comment lines
        string_split_by_delimeter(line, '#', true);
        string_trim(line);

        // ignore empty lines
        if (line.size() < 1)
            continue;

        // if line has a ':' then it has a label
        if (line.find_first_of(':') != std::string::npos) {
            // get label and check for spaces (spaces are not allowed in labels)
            string_split_by_delimeter(line, ':', true);
            if (line.find_first_of(' ') != std::string::npos) {
                std::cerr << "ERROR: Label with spaces defined at " << line_count << "." << std::endl;
                exit(1);
            }

            string_trim(line);
            if (line.size() < 1) {
                std::cerr << "ERROR: Empty label defined at line " << line_count << "." << std::endl;
                exit(1);
            }

            // if we find a label that is already in the 'labels' map, we got a duplicate label
            if (labels.contains(line)) {
                std::cerr << "ERROR: Duplicate label definition:" << std::endl;
                std::cerr << "    Label '" << line << "' defined at line " << labels.at(line).line_number << ", was redefined at line " << line_count << "." << std::endl;
                exit(1);
            }

            labels.emplace(std::move(line), Label {.points_to = (void*)inst_count, .line_number = line_count});
        } else {
            // if there is no label or comment and it is not en empty line, we have an instruction
            inst_count++;
        }
    }

    // make file ptr point back top the beginning
    file.clear();
    file.seekg(0, std::ios::beg);
}

void Program::parse_from_file(const char *path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "ERROR: An error occured when trying to read a program asm from '" << path << "'." << std::endl;
        exit(1);
    }

    // hash table to keep track of all labels in the source code
    std::unordered_map<std::string, Label> labels;

    // get all the labels and the addrs they point to
    get_label_definitions(file, labels);

    size_t line_count = 0;
    std::string line;
    while (std::getline(file, line)) {
        line_count++;

        // ignore comments in same line as instruction and ignore comment lines
        string_split_by_delimeter(line, '#', true);
        // ignore label in same line
        string_split_by_delimeter(line, ':', false);
        string_trim(line);

        // ignore empty lines
        if (line.size() < 1)
            continue;

        std::string operand = "";
        std::string instruction = "";
        const size_t space_index = line.find_first_of(' ');
        if (space_index == std::string::npos) {
            // no argument was passed to the instruction
            instruction = std::move(line);
        } else {
            // parse the argument and instructions passed in
            instruction = line.substr(0, space_index);
            operand = line.substr(space_index, line.size() - space_index); // operand does not need trimming
            string_trim(operand); // needs to be trimmed because of the label implementation system
        }

        // loop over all the instructions
        bool parsed_inst = false;
        for (int inst_to_check = 0; inst_to_check < Inst_Type::INST_COUNT; inst_to_check++) {
            // compare the instructions
            if (instruction.compare(inst_type_as_cstr((Inst_Type)inst_to_check)) != 0)
                continue;

            parsed_inst = true;
            Inst new_inst;
            // parse the operands
            if (inst_requires_operand((Inst_Type)inst_to_check) == true) {
                if (operand != "") {
                    // fill in the new instruction
                    new_inst.type = (Inst_Type) inst_to_check;

                    // we assume that any instruction that accepts a label, will have an operand of type 'void*'
                    if (inst_operand_might_be_label((Inst_Type)inst_to_check)) {
                        // check if operand is a label and it is defined
                        // WARNING: this does not block the  user from using labels that are just numbers (maybe that should be changed in the future)
                        if (labels.contains(operand)) {
                            new_inst.operand = Nan_Box(labels.at(operand).points_to);
                        } else if (std::all_of(operand.begin(), operand.end(), ::isdigit)) {
                            // if the operand is not in the labels map, it needs to be a number(absolute address)
                            // this check also makes sure we dont get any negative addresses
                            new_inst.operand = Nan_Box((void*)std::stol(operand));
                        } else {
                            // operand is an invalid addr or label
                            std::cerr << "ERROR: Invalid address or undefind label '" << operand << "' at line " << line_count << "." << std::endl;
                            exit(1);
                        }
                    } else if (inst_accepts_fp_operand((Inst_Type)inst_to_check)) {
                        char *end_ptr = nullptr;
                        new_inst.operand = Nan_Box(std::strtol(operand.c_str(), &end_ptr, 10));
                        if ((size_t)(end_ptr - operand.c_str()) != operand.size()) {
                            // replace ',' with '.'
                            const size_t comma_index = operand.find_first_of(',');
                            if (comma_index != std::string::npos)
                                operand.replace(comma_index, 1, ".");

                            // could not parse as int, try now as double
                            new_inst.operand = Nan_Box(std::strtod(operand.c_str(), &end_ptr));
                            if ((size_t)(end_ptr - operand.c_str()) != operand.size()) {
                                std::cerr << "ERROR: Invalid operand '" << operand << "' at line " << line_count << "." << std::endl;
                                exit(1);
                            }
                        }
                    } else if (inst_operand_might_be_function((Inst_Type)inst_to_check)) {
                        // loop over all the native functions
                        bool found = false;
                        for (size_t i = 0; i < Vm::native_funcs_count; i++) {
                            if (operand == Vm::native_funcs_names[i]) {
                                new_inst.operand = Nan_Box(i);
                                found = true;
                            }
                        }

                        if (!found) {
                            std::cerr << "ERROR: Invalid native function call to '" << operand << "'." << std::endl;
                            exit(1);
                        }
                    } else {
                        // TODO: replace with c functions to avoid exceptions
                        // TODO: in some cases, we can detect if the operand should not be negative (at compile time)
                        // implement that
                        try {
                            size_t char_processed_count = 0;
                            new_inst.operand = Nan_Box(std::stol(operand, &char_processed_count));

                            // operand was not entirely processed
                            if (char_processed_count != operand.size()) {
                                std::cerr << "ERROR: Invalid operand '" << operand << "' at line " << line_count << "." << std::endl;
                                exit(1);
                            }
                        } catch(const std::exception& e) {
                            std::cerr << "ERROR: Invalid operand '" << operand << "' at line " << line_count << "." << std::endl;
                            exit(1);
                        }
                    }
                    /*// WARNING: this assumes that any instrcution that does not accept a label or a floating point number, also, does not accept negative numbers
                    // maybe this should be changed in the future
                    } else if (std::all_of(operand.begin(), operand.end(), ::isdigit)) {
                        // convert operand to int
                        new_inst.operand = Nan_Box(std::stol(operand));
                    } else {
                        std::cerr << "ERROR: Invalid operand '" << operand << "' at line " << line_count << "." << std::endl;
                        exit(1);
                    }*/
                } else {
                    std::cerr << "ERROR: An error occured when parsing asm file:" << std::endl;
                    std::cerr << "    Instruction '" << inst_type_as_cstr((Inst_Type)inst_to_check) << "' on line " << line_count << " requires an operand." << std::endl;
                    exit(1);
                }
            } else {
                if (operand == "") {
                    // fill in the new instruction
                    new_inst.type = (Inst_Type)inst_to_check;
                } else {
                    std::cerr << "ERROR: An error occured when parsing asm file:" << std::endl;
                    std::cerr << "    Instruction '" << inst_type_as_cstr((Inst_Type)inst_to_check) << "' on line " << line_count << " does not require an operand." << std::endl;
                    exit(1);
                }
            }

            // add the instruction
            insts.push_back(new_inst);
            break;
        }

        if (parsed_inst == false) {
            std::cerr << "ERROR: Unknown instruction '" << instruction <<  "' on line " << line_count <<"." << std::endl;
            exit(1);
        }
    }

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
