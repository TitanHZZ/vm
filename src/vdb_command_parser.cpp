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
    else if (str == "delete")   return Vdb_Command_Type::DELETE;
    else if (str == "x")        return Vdb_Command_Type::X;
    else return Vdb_Command_Type::UNKNOWN;
}

const char *Vdb_Cmd_Parser::tk_type_as_str(Vdb_Token_Type type) {
    switch (type) {
    case Vdb_Token_Type::COMMAND: return "string";
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
            const std::string number = read_while(str, [](char c){ return std::isdigit(c); });
            tokens.push_back({std::move(number), Vdb_Token_Type::NUMBER});

        } else if (std::isalpha(str[pos])) {
            // handle keywords
            const std::string keyword = read_while(str, [](char c){ return std::isalpha(c); });
            if (str_as_vdb_cmd(keyword) != Vdb_Command_Type::UNKNOWN) {
                tokens.push_back({std::move(keyword), Vdb_Token_Type::COMMAND});
            } else {
                tokens.push_back({std::move(keyword), Vdb_Token_Type::UNKNOWN});
            }

        } else {
            // handle unknown keywords
            const std::string val = read_while(str, [](char c) { return !std::isspace(c); });
            tokens.push_back({std::move(val), Vdb_Token_Type::UNKNOWN});
        }
    }

    return tokens;
}

/*
 * parser funcs
 */
const Vdb_Token Vdb_Cmd_Parser::next(const std::vector<Vdb_Token> &tokens, const Vdb_Token_Type acc_type) {
    if (pos == tokens.size() - 1) {
        std::cerr << "ERROR: Expected command argument of type `" << tk_type_as_str(acc_type) << "` but found nothing." << std::endl;
        return Vdb_Token {std::string(), Vdb_Token_Type::UNKNOWN};
    }

    if (tokens[++pos].type != acc_type) {
        std::cerr << "ERROR: Expected token of type `" << tk_type_as_str(acc_type) << "` but found `" << tokens[pos].value << "`." << std::endl;
        return Vdb_Token {std::string(), Vdb_Token_Type::UNKNOWN};
    }

    return tokens[pos];
}

Vdb_Command Vdb_Cmd_Parser::parse_command(const std::vector<Vdb_Token> &tokens) {
    switch (str_as_vdb_cmd(tokens[pos].value)) {
    case Vdb_Command_Type::RUN: {
        // handle the run command
        return Vdb_Command { Vdb_Command_Type::RUN, std::vector<std::string>() };
    }

    case Vdb_Command_Type::BREAK: {
        // handle the break command
        const Vdb_Token &number = next(tokens, Vdb_Token_Type::NUMBER);
        if (number.type == Vdb_Token_Type::UNKNOWN) {
            return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<std::string>() };
        }

        return Vdb_Command { Vdb_Command_Type::BREAK, std::vector<std::string>(1, std::move(number.value)) };
    }

    case Vdb_Command_Type::NI:
    case Vdb_Command_Type::DISAS:
    case Vdb_Command_Type::INFO:
    case Vdb_Command_Type::DELETE:
    case Vdb_Command_Type::X:
    case Vdb_Command_Type::NOTHING:
    case Vdb_Command_Type::UNKNOWN:
    default:
        return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<std::string>() };
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

        case Vdb_Token_Type::NUMBER:
        case Vdb_Token_Type::UNKNOWN:
        default:
            std::cout << "Unknown command `" << tokens[pos].value << "`." << std::endl;
            return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<std::string>() };
        }
    } else {
        return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<std::string>() };
    }

    pos++;
    if (tokens.size() - pos > 0) {
        std::cout << "ERROR: Malformed command, unexpected token `" << tokens[pos].value << "`." << std::endl;
        return Vdb_Command { Vdb_Command_Type::NOTHING, std::vector<std::string>() };
    }

    return res;
}
