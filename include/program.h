#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <unordered_map>

#include "inst.h"

typedef struct {
    void *points_to;
    uint64_t line_number;
    std::string file_name;
} Label;

typedef struct {
    size_t insts_idx;
    std::string label_name;
} Unresolved_Label;

class Program {
private:
    void string_trim(std::string& str);
    void string_split_by_delimeter(std::string& line, const char ch, const bool get_first_substring);
    void get_label_definition(std::string& line, const char *const path, std::unordered_map<std::string, Label>& labels, const size_t line_count);
    void parse_source_code(const char *path, std::unordered_map<std::string, Label>& labels, std::unordered_map<std::string, std::string>& preprocessor_defines);
    void get_preprocessor_directive(std::string& line, const char *const path, std::unordered_map<std::string, Label>& labels, std::unordered_map<std::string, std::string>& preprocessor_defines, const size_t line_count);

public:
    std::vector<Inst> insts;
    std::vector<Nan_Box> memory;

    Program() {}
    Program(const Inst *list, const size_t count);
    // Program(Inst (&&inst_list)[], const size_t count)
    ~Program() {}

    static void write_to_file(const char *path, const Inst *program, const size_t program_size);
    void write_to_file(const char *path);
    void read_from_file(const char *path);
    void parse_from_file(const char *path);
    void print_program(bool with_labels = false);

    // basic program serialization
    friend std::ofstream& operator<<(std::ofstream& ofs, const Program& program) {
        // write the memory size to the binary file
        const size_t mem_size = program.memory.size();
        ofs.write((const char *)&mem_size, static_cast<std::streamsize>(sizeof(mem_size)));

        // write the memory to the binary file
        ofs.write((const char *)program.memory.data(), static_cast<std::streamsize>(sizeof(Nan_Box) * mem_size));

        // write the instructions size to the binary file
        const size_t insts_size = program.insts.size();
        ofs.write((const char *)&insts_size, static_cast<std::streamsize>(sizeof(insts_size)));

        // write the instructions size to the binary file
        ofs.write((const char *)program.insts.data(), static_cast<std::streamsize>(sizeof(Inst) * insts_size));
        return ofs;
    }

    // basic program deserialization
    friend std::ifstream& operator>>(std::ifstream& ifs, Program& program) {
        // read the memory size from the binary file
        size_t mem_size;
        ifs.read((char *)&mem_size, sizeof(mem_size));

        // read the memory from the binary file
        Nan_Box new_nan_box;
        program.memory.reserve(mem_size); // reserve the size necessary to store all the static memory data
        for (size_t i = 0; i < mem_size; i++) {
            ifs.read((char *)&new_nan_box, sizeof(Nan_Box));
            program.memory.push_back(new_nan_box);
        }

        // read the instructions size from the binary file
        size_t insts_size;
        ifs.read((char *)&insts_size, sizeof(insts_size));

        // read the instructions from the binary file
        Inst new_inst;
        program.insts.reserve(insts_size); // reserve the size necessary to store all the instructions
        for (size_t i = 0; i < insts_size; i++) {
            ifs.read((char *)&new_inst, sizeof(Inst));
            program.insts.push_back(new_inst);
        }

        return ifs;
    }
};
