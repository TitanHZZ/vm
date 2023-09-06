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

class Program {
private:
    void string_trim(std::string& str);
    void string_split_by_delimeter(std::string& line, const char ch, const bool get_first_substring);
    void get_label_definitions(std::ifstream& file, const char *path, std::unordered_map<std::string, Label>& labels);
    void parse_source_code(const char *path, std::unordered_map<std::string, Label>& labels, std::unordered_map<std::string, std::string>& preprocessor_defines);

public:
    std::vector<Inst> insts;

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
