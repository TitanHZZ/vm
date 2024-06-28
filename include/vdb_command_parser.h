#pragma once
#include <string>
#include <vector>
#include <functional>

/*
 * Command Parsing related structures and enums.
 */
enum class Vdb_Command_Type {
    RUN = 0,
    NI,
    DISAS,
    BREAK,
    INFO,
    DELETE,
    X,

    NOTHING, // used when user types a malformed command or stdin is empty/contains just spaces
    UNKNOWN  // user types a command that does not exist
};

struct Vdb_Command {
    Vdb_Command_Type type;
    std::vector<std::string> args;
};

/*
 * Vdb Lexer related structs and enums.
 */
enum class Vdb_Token_Type {
    COMMAND = 0,
    NUMBER,
    UNKNOWN
};

struct Vdb_Token {
    std::string value;
    Vdb_Token_Type type;
};

class Vdb_Cmd_Parser {
public:
    Vdb_Cmd_Parser()  {};
    ~Vdb_Cmd_Parser() {};

    // parser funcs
    Vdb_Command get_vdb_command(const std::string &str);

private:
    // aux funcs
    Vdb_Command_Type str_as_vdb_cmd(const std::string &str);
    const char *tk_type_as_str(Vdb_Token_Type type);

    // lexer funcs
    const std::string read_while(const std::string &str, const std::function<bool(char)> &predicate);
    std::vector<Vdb_Token> vdb_lexer(const std::string &str);

    // parser funcs
    const Vdb_Token next(const std::vector<Vdb_Token> &tokens, const Vdb_Token_Type acc_type);
    Vdb_Command parse_command(const std::vector<Vdb_Token> &tokens);

    size_t pos = 0;
};
