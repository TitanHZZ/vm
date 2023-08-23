#include <iostream>
#include <stdint.h>
#include <vector>
#include <fstream>

#include "exceptions.h"
#include "vm.h"
#include "program.h"

#define MAKE_INST_NOP()       (Inst) {.type = Inst_Type::INST_NOP, .operand = 0}
#define MAKE_INST_HALT()      (Inst) {.type = Inst_Type::INST_HALT, .operand = 0}
#define MAKE_INST_EXIT()      (Inst) {.type = Inst_Type::INST_EXIT, .operand = 0}
#define MAKE_INST_PUSH(val)   (Inst) {.type = Inst_Type::INST_PUSH, .operand = val}
#define MAKE_INST_POP()       (Inst) {.type = Inst_Type::INST_POP, .operand = 0}
#define MAKE_INST_ADD()       (Inst) {.type = Inst_Type::INST_ADD, .operand = 0}
#define MAKE_INST_SUB()       (Inst) {.type = Inst_Type::INST_SUB, .operand = 0}
#define MAKE_INST_MUL()       (Inst) {.type = Inst_Type::INST_MUL, .operand = 0}
#define MAKE_INST_DIV()       (Inst) {.type = Inst_Type::INST_DIV, .operand = 0}
#define MAKE_INST_JMP(val)    (Inst) {.type = Inst_Type::INST_JMP, .operand = val}
#define MAKE_INST_EQU()       (Inst) {.type = Inst_Type::INST_EQU, .operand = 0}
#define MAKE_INST_JMP_IF(val) (Inst) {.type = Inst_Type::INST_JMP_IF, .operand = val}
#define MAKE_INST_DUP(val)    (Inst) {.type = Inst_Type::INST_DUP, .operand = val}

// count up to 50
const Inst program2[] = {
    MAKE_INST_PUSH(0),
    MAKE_INST_PUSH(1),
    MAKE_INST_ADD(),
    MAKE_INST_DUP(0),
    MAKE_INST_PUSH(50),
    MAKE_INST_EQU(),
    MAKE_INST_JMP_IF(8),
    MAKE_INST_JMP(1),
    MAKE_INST_POP(),
    MAKE_INST_EXIT()
};

// generates fibonacci sequence
const Inst program[] = {
    MAKE_INST_PUSH(0),
    MAKE_INST_PUSH(1),
    MAKE_INST_DUP(1),
    MAKE_INST_DUP(1),
    MAKE_INST_ADD(),
    MAKE_INST_JMP(2),
};
constexpr size_t program_size = sizeof(program) / sizeof(program[0]);

Vm vm;
Program p;
// Program p(program, program_size);
// Program p(std::move(program), program_size); // takes ownership of the program

int main(int, char**) {
    vm.execute_program("../test.vasm");
    return 0;
}
