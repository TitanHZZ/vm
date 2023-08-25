#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>

#include "inst.h"

typedef struct {
    Word points_to;
    Word line_number;
} Label;

struct Program {
    std::vector<Inst> insts;

    Program() {}
    Program(const Inst *list, const size_t count) {
        for (size_t i = 0; i < count; i++) {
            insts.push_back(list[i]);
        }
    }
    /*Program(Inst (&&inst_list)[], const size_t count) {
        for (size_t i = 0; i < count; i++) {
            insts.push_back(std::move(inst_list[i]));
        }
    }*/

    ~Program() {}

    static void write_to_file(const char *path, const Inst *program, const size_t program_size) {
        std::ofstream file;
        file.open(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "ERROR: An error occured when trying to write a program to '" << path << "'" << std::endl;
            exit(1);
        }

        file.write((const char *)program, sizeof(Inst) * program_size);
        file.close();
    }

    void write_to_file(const char *path) {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "ERROR: An error occured when trying to write a program to '" << path << "'" << std::endl;
            exit(1);
        }

        file << *this;
        file.close();
    }

    void read_from_file(const char *path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "ERROR: An error occured when trying to read a program from '" << path << "'" << std::endl;
            exit(1);
        }

        file >> *this;
        file.close();
    }

    void string_trim(std::string& str) {
        // trim string
        while (str.size() > 0 && str.at(0) == ' ') str.erase(str.begin());      // trim left
        while (str.size() > 0 && str.at(str.size() - 1) == ' ') str.pop_back(); // trim right
    }

    void string_split_by_delimeter(std::string& line, const char ch, const bool get_first_substring) {
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

    void get_label_definitions(std::ifstream& file, std::unordered_map<std::string, Label>& labels) {
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

                labels.emplace(std::move(line), Label {.points_to = inst_count, .line_number = line_count});
            } else {
                // if there is no label or comment and it is not en empty line, we have an instruction
                inst_count++;
            }
        }

        // make file ptr point back top the beginning
        file.clear();
        file.seekg(0, std::ios::beg);
    }

    void parse_from_file(const char *path) {
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

                        // operand points to a label already defined
                        if (labels.contains(operand)) {
                            new_inst.operand = labels.at(operand).points_to;
                        } else {
                            try {
                                new_inst.operand = std::stoul(operand);
                            } catch(const std::exception& e) {
                                std::cerr << "ERROR: Undefined label '" << operand << "' at line " << line_count << "." << std::endl;
                                exit(1);
                            }
                        }
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

    // basic program serialization
    friend std::ofstream& operator<<(std::ofstream& ofs, const Program& program) {
        ofs.write((const char *)program.insts.data(), sizeof(Inst) * program.insts.size());
        return ofs;
    }

    // basic program deserialization
    friend std::ifstream& operator>>(std::ifstream& ifs, Program& program) {
        ifs.seekg(0, std::ios::end); // seek to end of file
        const int file_size = ifs.tellg(); // size in bytes

        if (file_size == -1) {
            std::cout << "ERROR: Could not read file." << std::endl;
            exit(1);
        }
        ifs.seekg(0, std::ios::beg);

        if (file_size % sizeof(Inst) != 0) {
            std::cerr << "ERROR: Invalid file size. File is currupted." << std::endl;
            exit(1);
        }

        const int inst_count = file_size / sizeof(Inst);
        program.insts.reserve((size_t)inst_count); // reserve the size necessary to store all the instructions
        for (int i = 0; i < inst_count; i++) {
            Inst new_inst;
            ifs.read((char *)&new_inst, sizeof(Inst));
            program.insts.push_back(new_inst);
        }

        return ifs;
    }
};

typedef struct Program Program;
