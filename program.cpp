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

void Program::parse_from_file(const char *path) {
    // hash table to keep track of all labels in the source code
    std::unordered_map<std::string, Label> labels;

    // keep track of preprocessor defines
    std::unordered_map<std::string, std::string> preprocessor_defines;

    this->parse_source_code(path, labels, preprocessor_defines);
}

void Program::get_label_definition(std::string& line, const char *const path, std::unordered_map<std::string, Label>& labels, const size_t line_count) {
    // get label and check for spaces (spaces are not allowed in labels)
    string_split_by_delimeter(line, ':', true);
    if (line.find_first_of(' ') != std::string::npos) {
        std::cerr << "ERROR: Label with spaces was defined." << std::endl;
        std::cerr << "      " << path << ":" << line_count << std::endl;
        exit(1);
    }

    string_trim(line);
    if (line.size() < 1) {
        std::cerr << "ERROR: Empty label was defined." << std::endl;
        std::cerr << "      " << path << ":" << line_count << std::endl;
        exit(1);
    }

    // if we find a label that is already in the 'labels' map, we got a duplicate label
    if (labels.contains(line)) {
        std::cerr << "ERROR: Duplicate label definition:" << std::endl;
        if (labels.at(line).file_name == path) {
            // label was redefined in the same file as the 'original' label
            std::cerr << "      Label '" << line << "' defined at line " << labels.at(line).line_number << ", was redefined at line " << line_count << "." << std::endl;
            std::cerr << "      " << path << std::endl;
        } else {
            std::cerr << "      Previously defined at " << labels.at(line).file_name << ":" << labels.at(line).line_number << std::endl;
            std::cerr << "      Redefined at " << path << ":" << line_count << std::endl;
        }

        exit(1);
    }

    labels.emplace(std::move(line), Label {.points_to = (void*)insts.size(), .line_number = line_count, .file_name = path});
}

void Program::get_preprocessor_directive(std::string& line, const char *const path, std::unordered_map<std::string, Label>& labels, std::unordered_map<std::string, std::string>& preprocessor_defines, const size_t line_count) {
    string_split_by_delimeter(line, '%', false);
    string_trim(line);

    const size_t directive_first_space_idx = line.find_first_of(' ');
    std::string directive_operation = line.substr(0, directive_first_space_idx);
    if (directive_operation == "define") {
        // found 'define' directive, parse it
        const size_t directive_last_space_idx = line.find_last_of(' ');
        std::string directive_name = line.substr(directive_first_space_idx + 1, directive_last_space_idx - directive_first_space_idx - 1);
        string_trim(directive_name);
        const std::string directive_value = line.substr(directive_last_space_idx + 1, line.size() - directive_last_space_idx - 1);

        // add the define to the map
        if (preprocessor_defines.contains(directive_name)) {
            std::cerr << "ERROR: Duplicate preprocessor define '" << directive_name << "'." << std::endl;
            std::cerr << "      " << path << ":" << line_count << std::endl;
            exit(1);
        }

        preprocessor_defines.emplace(std::move(directive_name), std::move(directive_value));
    } else if (directive_operation == "include") {
        // found 'include' directive, parse it
        std::string directive_path = line.substr(directive_first_space_idx + 1, line.size() - directive_first_space_idx - 1);
        // remove quotes if they exist
        std::replace(directive_path.begin(), directive_path.end(), '"', ' ');
        string_trim(directive_path);

        // parse the file recursively
        this->parse_source_code(directive_path.c_str(), labels, preprocessor_defines);
    } else if (directive_operation == "res") {
        // this directive is used to reserve memory (static memory)
        const size_t directive_last_space_idx = line.find_last_of(' ');
        std::string directive_name = line.substr(directive_first_space_idx + 1, directive_last_space_idx - directive_first_space_idx - 1);
        string_trim(directive_name);
        const std::string directive_value = line.substr(directive_last_space_idx + 1, line.size() - directive_last_space_idx - 1);

        // check if the preprocessor 'res' name is already defined
        if (preprocessor_defines.contains(directive_name)) {
            std::cerr << "ERROR: Duplicate preprocessor res '" << directive_name << "'." << std::endl;
            std::cerr << "      " << path << ":" << line_count << std::endl;
            exit(1);
        }

        char *end_ptr = nullptr;
        const int64_t value = static_cast<int64_t>(std::strtoll(directive_value.c_str(), &end_ptr, 10));
        if ((size_t)(end_ptr - directive_value.c_str()) != directive_value.size()) {
            std::cerr << "ERROR: Invalid preprocessor directive operand '" << directive_value << "'." << std::endl;
            std::cerr << "      " << path << ":" << line_count << std::endl;
            exit(1);
        }

        if (value < 0) {
            std::cerr << "ERROR: Invalid preprocessor directive operand '" << directive_value << "'." << std::endl;
            std::cerr << "      Cannot reserve a negative amount of memory." << std::endl;
            std::cerr << "      " << path << ":" << line_count << std::endl;
            exit(1);
        }

        // add a new define to the map (with the addr of the requested memory block)
        preprocessor_defines.emplace(std::move(directive_name), std::to_string(memory.size()));

        // reserve the requested memory amount (initialized with 0)
        for (int64_t i = 0; i < value; i++) {
            memory.push_back(Nan_Box(static_cast<int64_t>(0)));
        }
    } else if (directive_operation == "string") {
        // this directive is used to reserve memory (static memory) and initialize it with a string
        std::string directive_name_values = line.substr(directive_first_space_idx + 1, line.size() - directive_first_space_idx - 1);
        string_trim(directive_name_values);

        const size_t directive_space_idx = directive_name_values.find_first_of(' ');
        std::string directive_name = directive_name_values.substr(0, directive_space_idx);
        // directive values should be a comma separated list of strings/bytes
        std::string directive_values = directive_name_values.substr(directive_space_idx + 1, directive_name_values.size() - directive_space_idx);

        // check if the preprocessor 'res' name is already defined
        if (preprocessor_defines.contains(directive_name)) {
            std::cerr << "ERROR: Duplicate preprocessor res '" << directive_name << "'." << std::endl;
            std::cerr << "      " << path << ":" << line_count << std::endl;
            exit(1);
        }

        // get all the values to fill in the string in the static memory
        std::vector<int8_t> values;
        size_t comma_idx = directive_values.find_first_of(',');
        bool last_item = comma_idx == std::string::npos;
        while (comma_idx != std::string::npos || last_item) {
            // get new directive value and add it to the vector
            std::string new_value = directive_values.substr(0, comma_idx);
            string_trim(new_value);

            // check if the value is a string literal or a byte
            if (new_value.find_first_of('"') != std::string::npos) {
                if (!new_value.starts_with('"') || !new_value.ends_with('"')) {
                    std::cerr << "ERROR: Preprocessor directive value has unmatched '\"'." << std::endl;
                    std::cerr << "      " << path << ":" << line_count << std::endl;
                    exit(1);
                }

                std::replace(new_value.begin(), new_value.end(), '"', ' ');
                new_value = new_value.substr(1, new_value.size() - 2); // ignore spaces introduced by removing the quotes

                // convert string to bytes and add them to the vector
                const char *str_ptr = new_value.c_str();
                while (*str_ptr != '\0') {
                    values.emplace_back(*str_ptr);
                    str_ptr++;
                }
            } else if (std::all_of(new_value.begin(), new_value.end(), ::isdigit)) {
                // value is a byte
                char *end_ptr = nullptr;
                uint8_t char_value = static_cast<uint8_t>(std::strtoull(new_value.c_str(), &end_ptr, 10));
                if ((size_t)(end_ptr - new_value.c_str()) != new_value.size()) {
                    std::cerr << "ERROR: Invalid number '" << new_value << "'." << std::endl;
                    std::cerr << "      " << path << ":" << line_count << std::endl;
                    exit(1);
                }

                values.emplace_back(char_value);
            } else {
                std::cerr << "ERROR: Invalid number '" << new_value << "'." << std::endl;
                std::cerr << "      " << path << ":" << line_count << std::endl;
                exit(1);
            }

            // processed last item, nothing else to do
            if (last_item)
                break;

            // 'remove' the value from the original string and find the next comma
            string_split_by_delimeter(directive_values, ',', false);
            comma_idx = directive_values.find_first_of(',');
            last_item = comma_idx == std::string::npos;
        }

        // add a new define to the map (with the addr of the requested memory block)
        preprocessor_defines.emplace(std::move(directive_name), std::to_string(memory.size()));

        // reserve the requested memory amount (initialized with 0)
        for (size_t i = 0; i < values.size(); i++) {
            memory.push_back(Nan_Box(static_cast<int64_t>(values[i])));
        }
    } else {
        std::cerr << "ERROR: Unknown preprocessor directive '" << directive_operation << "'." << std::endl;
        std::cerr << "      " << path << ":" << line_count << std::endl;
        exit(1);
    }
}

void Program::parse_source_code(const char *path, std::unordered_map<std::string, Label>& labels, std::unordered_map<std::string, std::string>& preprocessor_defines) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "ERROR: An error occured when trying to read a program asm from '" << path << "'." << std::endl;
        exit(1);
    }

    /*// get all the labels and the addrs they point to
    get_label_definitions(file, path, labels);*/

    // keep track of all the instructions that have labels that are not defined when they are referenced in the source code
    std::vector<Unresolved_Label> insts_with_undefined_labels;

    size_t line_count = 0;
    std::string line;
    while (std::getline(file, line)) {
        line_count++;

        // ignore comments in same line as instruction and ignore comment lines
        string_split_by_delimeter(line, '#', true);
        string_trim(line);

        // if line has a ':' then it has a label
        if (line.find_first_of(':') != std::string::npos) {
            this->get_label_definition(line, path, labels, line_count);
            string_trim(line);
        }

        // ignore empty lines
        if (line.size() < 1)
            continue;

        // check for preprocessor directives
        // preprocessor does not support multi line directives
        if (line.find_first_of('%') != std::string::npos) {
            // found preprocessor directive
            this->get_preprocessor_directive(line, path, labels, preprocessor_defines, line_count);
            continue;
        }

        /*// this makes sure that the preprocessor defines are applied
        for (auto& it: preprocessor_defines) {
            // make sure only one preprocessor define is applied per line
            // this should avoid problems with defines that are substrings of other defines
            if (line.find(it.first) != std::string::npos) {
                line.replace(line.find(it.first), it.first.size(), it.second);
                break;
            }
        }*/

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

                    // check if operand is a macroa and if it is, replace with the appropriate value
                    if (preprocessor_defines.contains(operand))
                        operand = preprocessor_defines.at(operand);

                    // we assume that any instruction that accepts a label, will have an operand of type 'void*'
                    if (inst_operand_might_be_label((Inst_Type)inst_to_check)) {
                        // check if operand is a label and it is defined
                        // WARNING: this does not block the  user from using labels that are just numbers (maybe that should be changed in the future)
                        if (labels.contains(operand)) {
                            new_inst.operand = Nan_Box(labels.at(operand).points_to);
                        } else if (std::all_of(operand.begin(), operand.end(), ::isdigit)) {
                            // if the operand is not in the labels map, it needs to be a number(absolute address)
                            // this check also makes sure we dont get any negative addresses
                            new_inst.operand = Nan_Box((void*)std::stoll(operand));
                        } else {
                            // we assume that this new instruction will be added to the 'insts' vector
                            // thats why we pass the vector size as the index of the instruction
                            insts_with_undefined_labels.push_back( Unresolved_Label {.insts_idx = insts.size(), .label_name = operand} );
                        }
                    } else if (inst_accepts_fp_operand((Inst_Type)inst_to_check)) {
                        char *end_ptr = nullptr;
                        new_inst.operand = Nan_Box(static_cast<int64_t>(std::strtoll(operand.c_str(), &end_ptr, 10)));
                        if ((size_t)(end_ptr - operand.c_str()) != operand.size()) {
                            // replace ',' with '.' if it exists
                            std::replace(operand.begin(), operand.end(), ',', '.');

                            // could not parse as int, try now as double
                            new_inst.operand = Nan_Box(std::strtod(operand.c_str(), &end_ptr));
                            if ((size_t)(end_ptr - operand.c_str()) != operand.size()) {
                                std::cerr << "ERROR: Invalid operand '" << operand << "'." << std::endl;
                                std::cerr << "      " << path << ":" << line_count << std::endl;
                                exit(1);
                            }
                        }
                    } else if (inst_operand_might_be_function((Inst_Type)inst_to_check)) {
                        // loop over all the native functions
                        bool found = false;
                        for (size_t i = 0; i < Vm::native_funcs_count; i++) {
                            if (operand == Vm::native_funcs_names[i]) {
                                new_inst.operand = Nan_Box(static_cast<int64_t>(i));
                                found = true;
                            }
                        }

                        if (!found) {
                            std::cerr << "ERROR: Invalid native function call to '" << operand << "'." << std::endl;
                            std::cerr << "      " << path << ":" << line_count << std::endl;
                            exit(1);
                        }
                    } else {
                        // TODO: replace with c functions to avoid exceptions
                        // TODO: in some cases, we can detect if the operand should not be negative (at compile time)
                        // implement that
                        try {
                            size_t char_processed_count = 0;
                            new_inst.operand = Nan_Box(static_cast<int64_t>(std::stoll(operand, &char_processed_count)));

                            // operand was not entirely processed
                            if (char_processed_count != operand.size()) {
                                std::cerr << "ERROR: Invalid operand '" << operand << "'." << std::endl;
                                std::cerr << "      " << path << ":" << line_count << std::endl;
                                exit(1);
                            }
                        } catch(const std::exception&) {
                            std::cerr << "ERROR: Invalid operand '" << operand << "'." << std::endl;
                            std::cerr << "      " << path << ":" << line_count << std::endl;
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
                    std::cerr << "      Instruction '" << inst_type_as_cstr((Inst_Type)inst_to_check) << "' requires an operand." << std::endl;
                    std::cerr << "      " << path << ":" << line_count << std::endl;
                    exit(1);
                }
            } else {
                if (operand == "") {
                    // fill in the new instruction
                    new_inst.type = (Inst_Type)inst_to_check;
                } else {
                    std::cerr << "ERROR: An error occured when parsing asm file:" << std::endl;
                    std::cerr << "      Instruction '" << inst_type_as_cstr((Inst_Type)inst_to_check) << "' does not require an operand." << std::endl;
                    std::cerr << "      " << path << ":" << line_count << std::endl;
                    exit(1);
                }
            }

            // add the instruction
            insts.push_back(new_inst);
            break;
        }

        if (parsed_inst == false) {
            std::cerr << "ERROR: Unknown instruction '" << instruction <<  "'." << std::endl;
            std::cerr << "      " << path << ":" << line_count << std::endl;
            exit(1);
        }
    }

    // fill in the addrs for instructions with undefined labels
    for (auto& it: insts_with_undefined_labels) {
        if (labels.contains(it.label_name)) {
            insts[it.insts_idx].operand = Nan_Box(labels.at(it.label_name).points_to);
        } else {
            // inst operand is an invalid addr or label
            std::cerr << "ERROR: Undefind label '" << it.label_name << "'." << std::endl;
            std::cerr << "      " << path << ":" << line_count << std::endl;
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
