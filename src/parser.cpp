// #include <iostream>

#include "parser.h"
#include "directive.h"
// #include "inst.h"
// #include "vm.h"

#define Includes  (parent == nullptr ? includes : parent->includes)
#define Insts     (parent == nullptr ? insts : parent->insts)
#define Labels    (parent == nullptr ? labels : parent->labels)
#define UnRLabels (parent == nullptr ? unresolved_labels : parent->unresolved_labels)
#define Alias     (parent == nullptr ? alias : parent->alias)
#define Memory    (parent == nullptr ? memory : parent->memory)

Parser::Parser(std::vector<Token> &tokens, Parser *parent) : pos(0), tokens(tokens), parent(parent) {
    // save the file path of the current file
    if (tokens.size() > 0) {
        Includes.insert(tokens[0].file_path);
        Insts.reserve(static_cast<size_t>(tokens.size() / 2)); // rough estimation of how many instructions we have
    }
}

void Parser::parse(Program *p) {
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

    if (parent == nullptr) {
        // second pass to resolve the remaining labels
        for (Unresolved_Label &ul: unresolved_labels) {
            if (!labels.contains(ul.token.value)) {
                std::cerr << "ERROR: " << ul.token.file_path << ":" << ul.token.line_number << ":" << ul.token.line_offset << ": Unresolved label \"" << ul.token.value << "\"." << std::endl;
                std::exit(1);
            }

            insts[ul.inst_idx].operand.box_ptr(labels[ul.token.value].points_to);
        }

        // move all the data to the program
        p->insts  = std::move(insts);
        p->memory = std::move(memory);
    }
}

void Parser::parse_inst() {
    const Inst_Type inst_type = str_as_inst(tokens[pos].value);

    if (!inst_requires_operand(inst_type)) {
        Insts.push_back(Inst {inst_type, Nan_Box()});
        return;
    }

    const Token &operand = next(inst_acc_tk[inst_type], sizeof(inst_acc_tk[0]) / sizeof(inst_acc_tk[0][0]));
    Insts.push_back(Inst {inst_type, token_as_Nan_Box(operand)});
}

void Parser::parse_label() {
    // check for duplicate label definition
    if (Labels.contains(tokens[pos].value)) {
        const Token *const tk = Labels[tokens[pos].value].token;
        std::cerr << "ERROR: Label \"" << tokens[pos].value << "\" was redefined." << std::endl;
        std::cerr << "\tInitially defined at " << tk->file_path << ":" << tk->line_number << ":" << tk->line_offset << "." << std::endl;
        std::cerr << "\tRedefined at " << tokens[pos].file_path << ":" << tokens[pos].line_number << ":" << tokens[pos].line_offset << "." << std::endl;
        std::exit(1);
    }

    const size_t l_size = parent == nullptr ? insts.size() : parent->insts.size();
    Labels[tokens[pos].value] = Label {(void *)l_size, &tokens[pos]};
}

void Parser::parse_directive() {
    switch (str_as_dir(tokens[pos].value)) {
    case Directive_Type::ALIAS: {
        const Token &name  = next(dir_acc_tk[ALIAS][0], sizeof(dir_acc_tk[ALIAS][0]) / sizeof(dir_acc_tk[ALIAS][0][0]), true);
        const Token &value = next(dir_acc_tk[ALIAS][1], sizeof(dir_acc_tk[ALIAS][1]) / sizeof(dir_acc_tk[ALIAS][1][0]));

        if (Alias.contains(name.value)) {
            const Token &na = Alias[name.value];
            std::cerr << "ERROR: Alias \"" << name.value << "\" was redefined." << std::endl;
            std::cerr << "\tInitially defined at " << na.file_path << ":" << na.line_number << ":" << na.line_offset << "." << std::endl;
            std::cerr << "\tRedefined at " << value.file_path << ":" << value.line_number << ":" << value.line_offset << "." << std::endl;
            std::exit(1);
        }

        Alias[name.value] = value;
        break;
    }

    case Directive_Type::INCLUDE: {
        const Token &include_path = next(dir_acc_tk[INCLUDE][0], sizeof(dir_acc_tk[INCLUDE][0]) / sizeof(dir_acc_tk[INCLUDE][0][0]));

        Lexer lexer(include_path.value.c_str());
        std::vector<Token> &new_tokens = lexer.tokenize();

        if (new_tokens.size() > 0 && Includes.find(new_tokens[0].file_path) != includes.end()) {
            // file was already included
            std::cerr << "ERROR: Circular file inclusion detected. The file \"" << new_tokens[0].file_path << "\" was included more than once." << std::endl;
            std::exit(1);
        }

        Parser parser(new_tokens, parent == nullptr ? this : parent);
        parser.parse();

        break;
    }

    case Directive_Type::STR: {
        const Token &name  = next(dir_acc_tk[STR][0], sizeof(dir_acc_tk[STR][0]) / sizeof(dir_acc_tk[STR][0][0]), true);
        const Token &value = next(dir_acc_tk[STR][1], sizeof(dir_acc_tk[STR][1]) / sizeof(dir_acc_tk[STR][1][0]));

        // save the addr of the string as an alias
        Token addr = name;
        addr.value = std::to_string(Memory.size());
        addr.type = Token_Type::INTEGER;
        Alias[name.value] = std::move(addr);

        // save the string the the memory
        Memory.reserve(value.value.size());
        for (const char &c: value.value) {
            Memory.push_back(Nan_Box(static_cast<int64_t>(c)));
        }

        break;
    }

    case Directive_Type::RES: {
        const Token &name  = next(dir_acc_tk[RES][0], sizeof(dir_acc_tk[RES][0]) / sizeof(dir_acc_tk[RES][0][0]), true);
        const Token &value = next(dir_acc_tk[RES][1], sizeof(dir_acc_tk[RES][1]) / sizeof(dir_acc_tk[RES][1][0]));

        // check the reserve size
        const int64_t size = std::strtoll(value.value.c_str(), nullptr, 10);
        if (size <= 0) {
            std::cerr << "ERROR: " << value.file_path << ":" << value.line_number << ":" << value.line_offset << ": Can only reserve an amount of memory positive and bigger than 0;" << std::endl;
            std::exit(1);
        }

        // save the addr of the memory block as an alias
        Token addr = name;
        addr.value = std::to_string(Memory.size());
        addr.type = Token_Type::INTEGER;
        Alias[name.value] = std::move(addr);

        // zero initialize the memory block
        Memory.reserve(static_cast<size_t>(size));
        for (size_t i = 0; i < static_cast<size_t>(size); i++) {
            Memory.push_back(Nan_Box(static_cast<int64_t>(0)));
        }

        break;
    }

    case Directive_Type::COUNT:
    default:
        std::cerr << "ERROR: " << tokens[pos].file_path << ":" << tokens[pos].line_number << ":" << tokens[pos].line_offset << ": Unknown directive \"" << tokens[pos].value << "\"." << std::endl;
        std::exit(1);
    }
}

Nan_Box Parser::token_as_Nan_Box(const Token &token) {
    switch (token.type) {
        case Token_Type::INTEGER:
            return Nan_Box(static_cast<int64_t>(std::strtoll(token.value.c_str(), nullptr, 10)));

        case Token_Type::FP:
            return Nan_Box(std::strtod(token.value.c_str(), nullptr));

        case Token_Type::KEYWORD:
            if (Alias.contains(token.value)) {
                return token_as_Nan_Box(Alias[token.value]);
            } else if (Labels.contains(token.value)) {
                return Nan_Box(Labels[token.value].points_to);
            } else {
                UnRLabels.push_back(Unresolved_Label {Insts.size(), token});
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

const Token &Parser::next(const Token_Type acc_types[], size_t acc_types_size, bool ignore_keyword_type_check) {
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
            if (acc_types[i] == Token_Type::KEYWORD && Alias.contains(tokens[pos].value) && !ignore_keyword_type_check) {
                for (size_t j = 0; j < acc_types_size && acc_types[j] != Token_Type::UNKNOWN; j++) {
                    if (acc_types[j] == Alias[tokens[pos].value].type) {
                        return tokens[pos];
                    }
                }

                // the alias value type is not compatible with the types accepted in the current instruction/operation
                std::cerr << "ERROR: " << tokens[pos].file_path << ":" << tokens[pos].line_number << ":" << tokens[pos].line_offset << ": KEYWORD has an invalid token type of " << Lexer::type_as_cstr(Alias[tokens[pos].value].type) << ".";
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
