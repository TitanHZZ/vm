#include <iostream>
#include <vector>
#include <unordered_set>

#include "lexer.h"
#include "inst.h"
#include "program.h"
#include "vm.h"
#include "directive.h"

class Parser {
public:
    Parser(std::vector<Token> &tokens, Parser *parent = nullptr) : pos(0), tokens(tokens), parent(parent) {
        // save the file path of the current file
        if (tokens.size() > 0) {
            if (parent == nullptr) {
                includes.insert(tokens[0].file_path);
            } else {
                parent->includes.insert(tokens[0].file_path);
            }
        }
    }

    std::vector<Inst> &parse() {
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

        // second pass to resolve the remaining labels
        for (Unresolved_Label &ul: unresolved_labels) {
            if (!labels.contains(ul.token.value)) {
                std::cerr << "ERROR: " << ul.token.file_path << ":" << ul.token.line_number << ":" << ul.token.line_offset << ": Unresolved label \"" << ul.token.value << "\"." << std::endl;
                std::exit(1);
            }

            insts[ul.inst_idx].operand.box_ptr(labels[ul.token.value].points_to);
        }

        return insts;
    }

private:
    void parse_inst() {
        const Inst_Type inst_type = str_as_inst(tokens[pos].value);

        if (!inst_requires_operand(inst_type)) {
            add_inst(Inst {inst_type, Nan_Box()});
            return;
        }

        const Token &operand = next(inst_acc_tk[inst_type], sizeof(inst_acc_tk[0]) / sizeof(inst_acc_tk[0][0]));
        add_inst(Inst {inst_type, token_as_Nan_Box(operand)});
    }

    void parse_label() {
        // check for duplicate label definition
        if (labels.contains(tokens[pos].value)) {
            std::cerr << "ERROR: Label \"" << tokens[pos].value << "\" was redefined." << std::endl;
            std::cerr << "\tInitially defined at " << labels[tokens[pos].value].token->file_path << ":" << labels[tokens[pos].value].token->line_number << ":" << labels[tokens[pos].value].token->line_offset << "." << std::endl;
            std::cerr << "\tRedefined at " << tokens[pos].file_path << ":" << tokens[pos].line_number << ":" << tokens[pos].line_offset << "." << std::endl;
            std::exit(1);
        }

        if (parent == nullptr) {
            labels[tokens[pos].value] = Label {(void *)insts.size(), &tokens[pos]};
        } else {
            parent->labels[tokens[pos].value] = Label {(void *)parent->insts.size(), &tokens[pos]};
        }
    }

    void parse_directive() {
        if (tokens[pos].value == "alias") {
            const Token &name  = next(dir_acc_tk[ALIAS][0], sizeof(dir_acc_tk[ALIAS][0]) / sizeof(dir_acc_tk[ALIAS][0][0]), true);
            const Token &value = next(dir_acc_tk[ALIAS][1], sizeof(dir_acc_tk[ALIAS][1]) / sizeof(dir_acc_tk[ALIAS][1][0]));

            if (alias.contains(name.value)) {
                std::cerr << "ERROR: Alias \"" << name.value << "\" was redefined." << std::endl;
                std::cerr << "\tInitially defined at " << alias[name.value].file_path << ":" << alias[name.value].line_number << ":" << alias[name.value].line_offset << "." << std::endl;
                std::cerr << "\tRedefined at " << value.file_path << ":" << value.line_number << ":" << value.line_offset << "." << std::endl;
                std::exit(1);
            }

            if (parent == nullptr) {
                alias[name.value] = value;
            } else {
                parent->alias[name.value] = value;
            }

        } else if (tokens[pos].value == "include") {
            const Token &include_path = next(dir_acc_tk[INCLUDE][0], sizeof(dir_acc_tk[INCLUDE][0]) / sizeof(dir_acc_tk[INCLUDE][0][0]));

            Lexer lexer(include_path.value.c_str());
            std::vector<Token> &new_tokens = lexer.tokenize();

            if (lexer.get_error_count(true) != 0)
                std::exit(1);

            if (new_tokens.size() > 0) {
                if ((parent != nullptr && parent->includes.find(new_tokens[0].file_path) != includes.end() ) || includes.find(new_tokens[0].file_path) != includes.end()) {
                    // file was already included
                    std::cerr << "ERROR: Circular file inclusion detected. The file \"" << new_tokens[0].file_path << "\" was included more than once." << std::endl;
                    std::exit(1);
                }
            }

            Parser parser(new_tokens, parent == nullptr ? this : parent);
            parser.parse();

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
                    return Nan_Box(labels[token.value].points_to);
                } else {
                    unresolved_labels.push_back(Unresolved_Label {insts.size(), token});
                }

                return Nan_Box();

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

    const Token &next(const Token_Type acc_types[], size_t acc_types_size, bool ignore_keyword_type_check = false) {
        if (pos == tokens.size() - 1) {
            std::cerr << "ERROR: Expected token but found nothing." << std::endl;
            std::exit(1);
        }

        pos++;
        for (size_t i = 0; i < acc_types_size && acc_types[i] != Token_Type::UNKNOWN; i++) {
            if (acc_types[i] == tokens[pos].type) {
                /*
                 When we have a keyword as the next token, if it is an alias,
                 we need to check if the value is valid in the context of the current instruction/operation.

                 The 'ignore_keyword_type_check' should be set to 'true' when asking for the next token during an 'alias' directive parsing.
                 This is the case because the 'recursive' KEYWORD type check should not happen in this case as we are parsing a label definition
                 for the first time or we have a redefinition, and in that case, 'token_as_Nan_Box' will catch this. This is the same reason the loop
                 checks if the label already exists, so that 'token_as_Nan_Box' catches the redefinitions as this function should only
                 get the next token and check if the type is an accepted one by the current instruction/operation.
                 */
                if (acc_types[i] == Token_Type::KEYWORD && alias.contains(tokens[pos].value) && !ignore_keyword_type_check) {
                    for (size_t j = 0; j < acc_types_size && acc_types[j] != Token_Type::UNKNOWN; j++) {
                        if (acc_types[j] == alias[tokens[pos].value].type) {
                            return tokens[pos];
                        }
                    }

                    // the alias value type is not compatible with the types accepted in the current instruction/operation
                    std::cerr << "ERROR: " << tokens[pos].file_path << ":" << tokens[pos].line_number << ":" << tokens[pos].line_offset << ": KEYWORD has an invalid token type of " << Lexer::type_as_cstr(alias[tokens[pos].value].type) << ".";
                    std::cerr << " Expected KEYWORD with one of this types:";
                    for (size_t i = 0; i < acc_types_size && acc_types[i] != Token_Type::UNKNOWN; i++) {
                        if (acc_types[i] != Token_Type::KEYWORD)
                            std::cerr << " " << Lexer::type_as_cstr(acc_types[i]);
                    }
                    std::cerr << std::endl;
                    std::exit(1);
                }

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

    inline void add_inst(Inst inst) {
        if (parent == nullptr) {
            insts.push_back(inst);
        } else {
            parent->insts.push_back(inst);
        }
    }

    size_t pos;
    std::vector<Token> &tokens;

    /*
     This data structures **need** to own the data as the Lexers and Parsers get destroyed in the recursive process
     of the 'include' directive. In the end, the first created Lexer and Parser, both need to have access to valid data.
     */
    std::vector<Inst> insts;
    std::vector<Unresolved_Label> unresolved_labels;
    std::unordered_map<std::string, Token> alias;
    std::unordered_map<std::string, Label> labels;

    // used to stores the names for the included files to avoid circular file inclusion
    std::unordered_set<std::string> includes;

    Parser *parent;
};

int main() {
    Lexer lexer("../../examples/funcs.vasm");
    std::vector<Token> &tokens = lexer.tokenize();

    if (lexer.get_error_count(true) != 0)
        return 1;

    Parser parser(tokens);
    std::vector<Inst> &insts = parser.parse();

    Program p;
    p.insts = insts;

    Vm vm;
    vm.execute_program(p);

    return 0;
}
