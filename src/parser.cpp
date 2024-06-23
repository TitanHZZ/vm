#include <iostream>
#include <vector>

#include "lexer.h"
#include "inst.h"
#include "program.h"
#include "vm.h"
#include "directive.h"

class Parser {
public:
    Parser(std::vector<Token> &tokens) : tokens(tokens), pos(0) {}

    std::vector<Inst> parse() {
        while (pos < tokens.size()) {
            switch (tokens[pos].type) {
            case Token_Type::INSTRUCTION:
                parse_inst(); break;

            case Token_Type::LABEL:
                parse_label(); break;

            case Token_Type::DIRECTIVE:
                parse_directive(); break;

            case Token_Type::KEYWORD:
            case Token_Type::INTEGER:
            case Token_Type::FP:
            case Token_Type::STRING:
            case Token_Type::UNKNOWN:
            default:
                std::cerr << "ERROR: Parsing of token with type " << Lexer::type_as_cstr(tokens[pos].type) << " not yet implemented." << std::endl;
                std::cerr << tokens[pos].line_number << std::endl;
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

        const Token &operand = next(inst_acc_tk[inst_type], sizeof(inst_acc_tk[0]) / sizeof(inst_acc_tk[0][0]));
        insts.push_back({inst_type, token_as_Nan_Box(operand)});
    }

    void parse_label() {
        // check for duplicate label definition
        if (labels.contains(tokens[pos].value)) {
            std::cerr << "ERROR: Label \"" << tokens[pos].value << "\" was redefined." << std::endl;
            std::cerr << "\tInitially defined at " << labels[tokens[pos].value].token->file_path << ":" << labels[tokens[pos].value].token->line_number << ":" << labels[tokens[pos].value].token->line_offset << "." << std::endl;
            std::cerr << "\tRedefined at " << tokens[pos].file_path << ":" << tokens[pos].line_number << ":" << tokens[pos].line_offset << "." << std::endl;
            std::exit(1);
        }

        labels[tokens[pos].value] = Label {(void *)pos, &tokens[pos]};
    }

    void parse_directive() {
        if (tokens[pos].value == "alias") {
            const Token &name  = next(dir_acc_tk[ALIAS][0], sizeof(dir_acc_tk[ALIAS][0]) / sizeof(dir_acc_tk[ALIAS][0][0]));
            const Token &value = next(dir_acc_tk[ALIAS][1], sizeof(dir_acc_tk[ALIAS][1]) / sizeof(dir_acc_tk[ALIAS][1][0]));

            if (alias.contains(name.value)) {
                std::cerr << "ERROR: Alias \"" << name.value << "\" was redefined." << std::endl;
                std::cerr << "\tInitially defined at " << alias[name.value].file_path << ":" << alias[name.value].line_number << ":" << alias[name.value].line_offset << "." << std::endl;
                std::cerr << "\tRedefined at " << value.file_path << ":" << value.line_number << ":" << value.line_offset << "." << std::endl;
                std::exit(1);
            }

            alias[name.value] = value;
        } else {
            std::cerr << "ERROR: Unimplemented directive." << std::endl;
            std::exit(1);
        }
    }

    Nan_Box token_as_Nan_Box(const Token &token) {
        switch (token.type) {
            case Token_Type::INTEGER:
                return Nan_Box(static_cast<int64_t>(std::strtoll(token.value.c_str(), nullptr, 10)));

            case Token_Type::FP:
                return Nan_Box(std::strtod(token.value.c_str(), nullptr));

            case Token_Type::KEYWORD:
                if (alias.contains(token.value)) {
                    return token_as_Nan_Box(alias[token.value]);
                } else if (labels.contains(token.value)) {
                    return token_as_Nan_Box(*labels[token.value].token);
                } else {
                    unresolved_labels.push_back(Unresolved_Label {pos, token});
                }

                // if (!alias.contains(token.value)) {
                //     std::cerr << "ERROR: " << token.file_path << ":" << token.line_number << ":" << token.line_offset << ": Keyword not yet defined." << std::endl;
                //     std::exit(1);
                // }

            case Token_Type::INSTRUCTION:
            case Token_Type::DIRECTIVE:
            case Token_Type::LABEL:
            case Token_Type::STRING:
            case Token_Type::UNKNOWN:
            default:
                break;
        }

        std::cerr << "ERROR: Building a Nan Box with a token of type " << Lexer::type_as_cstr(tokens[pos].type) << " not yet implemented." << std::endl;
        std::exit(1);
    }

    const Token &next(const Token_Type acc_types[], size_t acc_types_size) {
        if (pos == tokens.size() - 1) {
            std::cerr << "ERROR: Expected token but found nothing." << std::endl;
            std::exit(1);
        }

        pos++;
        for (size_t i = 0; i < acc_types_size && acc_types[i] != Token_Type::UNKNOWN; i++) {
            if (acc_types[i] == tokens[pos].type) {
                // we got our value with a desired type
                return tokens[pos];
            }
        }

        // we do not have a valid type
        std::cerr << "ERROR: " << tokens[pos].file_path << ":" << tokens[pos].line_number << ":" << tokens[pos].line_offset << ": Invalid token type " << Lexer::type_as_cstr(tokens[pos].type) << ".";
        std::cerr << " Expected a token of one of this types:";
        for (size_t i = 0; i < acc_types_size && acc_types[i] != Token_Type::UNKNOWN; i++) {
            std::cerr << " " << Lexer::type_as_cstr(acc_types[i]);
        }
        std::cerr << std::endl;
        std::exit(1);
    }

    std::vector<Token> &tokens;
    size_t pos;

    std::vector<Inst> insts;
    std::vector<Unresolved_Label> unresolved_labels;
    std::unordered_map<std::string_view, Token> alias;
    std::unordered_map<std::string_view, Label> labels;
};

int main() {
    Lexer lexer("./examples/e.vasm");
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
