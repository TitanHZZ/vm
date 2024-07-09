#include <iostream>

#include "vdb_command_parser.h"

/*
 * aux funcs
 */
Vdb_Command_Type Vdb_Cmd_Parser::str_as_vdb_cmd(const std::string &str) {
    if      (str == "run")      return Vdb_Command_Type::RUN;
    else if (str == "ni")       return Vdb_Command_Type::NI;
    else if (str == "disas")    return Vdb_Command_Type::DISAS;
    else if (str == "break")    return Vdb_Command_Type::BREAK;
    else if (str == "info")     return Vdb_Command_Type::INFO;
    else if (str == "help")     return Vdb_Command_Type::HELP;
    else if (str == "delete")   return Vdb_Command_Type::DELETE;
    else if (str == "x")        return Vdb_Command_Type::X;
    else return Vdb_Command_Type::UNKNOWN;
}

const char *Vdb_Cmd_Parser::tk_type_as_str(Vdb_Token_Type type) {
    switch (type) {
    case Vdb_Token_Type::COMMAND: return "command";
    case Vdb_Token_Type::KEYWORD: return "string";
    case Vdb_Token_Type::NUMBER:  return "number";
    case Vdb_Token_Type::UNKNOWN: return "UNKNOWN";
    default: return "UNKNOWN";
    }
}

/*
 * lexer funcs
 */
const std::string Vdb_Cmd_Parser::read_while(const std::string &str, const std::function<bool(char)> &predicate) {
    const size_t start = pos;
    while (pos < str.size() && predicate(str[pos])) pos++;
    return str.substr(start, pos - start);
}

std::vector<Vdb_Token> Vdb_Cmd_Parser::vdb_lexer(const std::string &str) {
    std::vector<Vdb_Token> tokens;

    pos = 0;
    while (pos < str.size()) {
        if (std::isspace(str[pos])) {
            // handle spaces
            pos++;

        } else if (std::isdigit(str[pos])) {
            // handle digits
            const std::string number = read_while(str, [](char c){ return std::isdigit(c) || c == 'x' || (c >= 'a' && c <= 'f'); });
            tokens.push_back({ Vdb_Token_Type::NUMBER, std::move(number) });

        } else if (std::isalpha(str[pos])) {
            // handle keywords
            const std::string keyword = read_while(str, [](char c){ return std::isalpha(c) || c == '_' || (c >= '0' && c <= '9'); });
            if (str_as_vdb_cmd(keyword) != Vdb_Command_Type::UNKNOWN) {
                tokens.push_back({ Vdb_Token_Type::COMMAND, std::move(keyword) });
            } else {
                tokens.push_back({ Vdb_Token_Type::KEYWORD, std::move(keyword) });
            }

        } else {
            // handle unknown keywords
            const std::string val = read_while(str, [](char c) { return !std::isspace(c); });
            tokens.push_back({ Vdb_Token_Type::UNKNOWN, std::move(val) });
        }
    }

    return tokens;
}

/*
 * parser funcs
 */
// TODO: remove code duplication
const Vdb_Token Vdb_Cmd_Parser::next(const std::vector<Vdb_Token> &tokens, const Vdb_Token_Type *acc_types) {
    if (pos == tokens.size() - 1) {
        std::cerr << "Expected command argument of one of the following types: ";
        for (size_t i = 0; i < cmd_type_acc_tk_type_element_count && acc_types[i] != Vdb_Token_Type::UNKNOWN; i++) {
            std::cout << "`" << tk_type_as_str(acc_types[i]) << "` ";
        }

        std::cout << "but found nothing." << std::endl;
        return Vdb_Token { Vdb_Token_Type::UNKNOWN, std::string() };
    }

    pos++;
    for (size_t i = 0; i < cmd_type_acc_tk_type_element_count; i++) {
        if (tokens[pos].type == acc_types[i])
            return tokens[pos];
    }

    std::cerr << "Expected command argument of one of the following types: ";
    for (size_t i = 0; i < cmd_type_acc_tk_type_element_count && acc_types[i] != Vdb_Token_Type::UNKNOWN; i++) {
        std::cout << "`" << tk_type_as_str(acc_types[i]) << "` ";
    }
    std::cout << "but found `" << tokens[pos].value << "`." << std::endl;
    return Vdb_Token { Vdb_Token_Type::UNKNOWN, std::string() };
}

Vdb_Command Vdb_Cmd_Parser::parse_command(const std::vector<Vdb_Token> &tokens) {
    switch (str_as_vdb_cmd(tokens[pos].value)) {
    case Vdb_Command_Type::RUN:
        // handle the run command
        return Vdb_Command { Vdb_Command_Type::RUN, std::vector<Vdb_Token>() };

    case Vdb_Command_Type::BREAK: {
        // handle the break command
        const Vdb_Token &arg = next(tokens, cmd_type_acc_tk_type[static_cast<int>(Vdb_Command_Type::BREAK)]);
        if (arg.type == Vdb_Token_Type::UNKNOWN) {
            return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<Vdb_Token>() };
        }

        return Vdb_Command { Vdb_Command_Type::BREAK, std::vector<Vdb_Token>(1, std::move(tokens[pos])) };
    }

    case Vdb_Command_Type::DISAS:
        // handle the disassemble command
        return Vdb_Command { Vdb_Command_Type::DISAS, std::vector<Vdb_Token>() };

    case Vdb_Command_Type::NI:
        // handle next instruction command
        return Vdb_Command { Vdb_Command_Type::NI, std::vector<Vdb_Token>() };

    case Vdb_Command_Type::HELP:
        // handle help command
        return Vdb_Command { Vdb_Command_Type::HELP, std::vector<Vdb_Token>() };

    case Vdb_Command_Type::INFO: {
        // handle info command
        const Vdb_Token &arg = next(tokens, cmd_type_acc_tk_type[static_cast<int>(Vdb_Command_Type::INFO)]);
        if (arg.type == Vdb_Token_Type::UNKNOWN) {
            return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<Vdb_Token>() };
        }

        return Vdb_Command { Vdb_Command_Type::INFO, std::vector<Vdb_Token>(1, std::move(tokens[pos])) };
    }

    case Vdb_Command_Type::DELETE:
    case Vdb_Command_Type::X:
    case Vdb_Command_Type::NOTHING:
    case Vdb_Command_Type::UNKNOWN:
    default:
        return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<Vdb_Token>() };
    }
}

Vdb_Command Vdb_Cmd_Parser::get_vdb_command(const std::string &str) {
    // get command tokens
    std::vector<Vdb_Token> tokens = vdb_lexer(str);

    pos = 0;
    Vdb_Command res;
    if (pos < tokens.size()) {
        switch (tokens[pos].type) {
        case Vdb_Token_Type::COMMAND: {
            res = parse_command(tokens);
            break;
        }

        case Vdb_Token_Type::KEYWORD:
        case Vdb_Token_Type::NUMBER:
        case Vdb_Token_Type::UNKNOWN:
        default:
            std::cout << "Unknown command `" << tokens[pos].value << "`." << std::endl;
            return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<Vdb_Token>() };
        }
    } else {
        return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<Vdb_Token>() };
    }

    pos++;
    if (tokens.size() - pos > 0) {
        std::cout << "Malformed command, unexpected token `" << tokens[pos].value << "`." << std::endl;
        return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<Vdb_Token>() };
    }

    return res;
}
