#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "vm.h"
#include "inst.h"

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

    void string_ignore_comment_in_line(std::string& str) {
        // no comment in the line
        if(!str.contains('#'))
            return;

        // return index of 'ch'
        const int comment_index = str.find_first_of('#');
        str = str.substr(0, comment_index);
    }

    void parse_from_file(const char *path) {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "ERROR: An error occured when trying to read a program asm from '" << path << "'" << std::endl;
            exit(1);
        }

        size_t line_count = 1;
        std::string line;
        while (std::getline(file, line)) {
            string_trim(line);

            // ignore empty lines and comment lines
            if (line.size() < 1 || line.starts_with('#'))
                continue;

            // ignore comments in same line as instruction
            string_ignore_comment_in_line(line);
            string_trim(line);

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
                        new_inst.type = (Inst_Type)inst_to_check;
                        new_inst.operand = std::stoul(operand);
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

            line_count++;
            if (parsed_inst == false) {
                std::cerr << "ERROR: Unknown instruction '" << instruction <<  "' on line" << line_count <<"." << std::endl;
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
