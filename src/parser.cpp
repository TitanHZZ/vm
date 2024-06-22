#include <iostream>
#include <vector>

#include "lexer.h"
#include "inst.h"
#include "program.h"
#include "vm.h"

class Parser {
public:
    Parser(std::vector<Token> &tokens) : tokens(tokens), pos(0) {}

    std::vector<Inst> parse() {
        while (pos < tokens.size()) {
            switch (tokens[pos].type) {
            case Token_Type::INSTRUCTION:
                parse_inst();
                break;

            case Token_Type::DIRECTIVE:
            case Token_Type::KEYWORD:
            case Token_Type::LABEL:
            case Token_Type::INTEGER:
            case Token_Type::FP:
            case Token_Type::STRING:
            case Token_Type::UNKNOWN:
            default:
                std::cerr << "ERROR: Parsing of token with type " << Lexer::type_as_cstr(tokens[pos].type) << " not yet implemented." << std::endl;
                std::exit(1);
            }

            pos++;
        }

        return insts;
    }

private:
    void parse_inst() {
        const Inst_Type inst_type = str_as_inst(tokens[pos].value);

        if (!inst_requires_operand(inst_type)) {
            insts.push_back({inst_type, Nan_Box()});
            return;
        }

        const Token &operand = next();

        // loop all the token types accepted as parameters to the current instruction
        for (size_t i = 0; i < sizeof(inst_acc_tk[0]) / sizeof(Token_Type::UNKNOWN) && inst_acc_tk[inst_type][i] != Token_Type::UNKNOWN; i++) {
            if (inst_acc_tk[inst_type][i] == operand.type) {
                // we got our argument
                insts.push_back({inst_type, get_curr_tk_value()});
                return;
            }
        }

        // we do not have a valid operand type
        std::cerr << "ERROR: Invalid operand of type " << Lexer::type_as_cstr(tokens[pos].type) << ". Expected an operand of one of this types:";
        for (size_t i = 0; i < sizeof(inst_acc_tk[0]) / sizeof(Token_Type::UNKNOWN); i++) {
            std::cerr << " " << Lexer::type_as_cstr(inst_acc_tk[inst_type][i]);
        }
        std::cerr << std::endl;
        std::exit(1);
    }

    Nan_Box get_curr_tk_value() {
        switch (tokens[pos].type) {
            case Token_Type::INTEGER:
                return Nan_Box(static_cast<int64_t>(std::strtoll(tokens[pos].value.c_str(), nullptr, 10)));

            case Token_Type::FP:
                break;

            case Token_Type::INSTRUCTION:
            case Token_Type::DIRECTIVE:
            case Token_Type::KEYWORD:
            case Token_Type::LABEL:
            case Token_Type::STRING:
            case Token_Type::UNKNOWN:
            default:
                break;
        }

        std::cerr << "ERROR: Parsing of token with type " << Lexer::type_as_cstr(tokens[pos].type) << " not yet implemented." << std::endl;
        std::exit(1);
    }

    const Token &next() {
        if (pos == tokens.size() - 1) {
            std::cerr << "ERROR: Expected token but found nothing." << std::endl;
            std::exit(1);
        }

        return tokens[++pos];
    }

    std::vector<Token> &tokens;
    size_t pos;

    std::vector<Inst> insts; // TEMPORARY
};

int main() {
    Lexer lexer("./examples/123i.vasm");
    std::vector<Token> &tokens = lexer.tokenize();

    if (lexer.print_errors() != 0)
        return 1;

    // lexer.print_tokens();

    Parser parser(tokens);
    std::vector<Inst> insts = parser.parse();

    Program p;
    p.insts = insts;

    Vm vm;
    vm.execute_program(p);

    return 0;
}
