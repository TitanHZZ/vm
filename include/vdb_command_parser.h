#pragma once
#include <string>
#include <vector>
#include <functional>

/*
 * Vdb Lexer related structs and enums.
 */
enum class Vdb_Token_Type {
    COMMAND = 0,
    NUMBER,
    KEYWORD,
    UNKNOWN
};

struct Vdb_Token {
    Vdb_Token_Type type;
    std::string value;
};

constexpr static Vdb_Token_Type cmd_type_acc_tk_type[][2] = {
    {Vdb_Token_Type::UNKNOWN, Vdb_Token_Type::UNKNOWN}, // RUN
    {Vdb_Token_Type::UNKNOWN, Vdb_Token_Type::UNKNOWN}, // NI
    {Vdb_Token_Type::UNKNOWN, Vdb_Token_Type::UNKNOWN}, // DISAS
    {Vdb_Token_Type::NUMBER,  Vdb_Token_Type::KEYWORD}, // BREAK
    {Vdb_Token_Type::COMMAND, Vdb_Token_Type::UNKNOWN}, // INFO
    {Vdb_Token_Type::UNKNOWN, Vdb_Token_Type::UNKNOWN}, // DELETE
    {Vdb_Token_Type::UNKNOWN, Vdb_Token_Type::UNKNOWN}, // X
    {Vdb_Token_Type::UNKNOWN, Vdb_Token_Type::UNKNOWN}, // NOTHING
    {Vdb_Token_Type::UNKNOWN, Vdb_Token_Type::UNKNOWN}  // UNKNOWN
};

/*
 * Vdb Parser related structures and enums.
 */
enum class Vdb_Command_Type {
    RUN = 0,
    NI,
    DISAS,
    BREAK,
    INFO,
    HELP,
    DELETE,
    X,

    NOTHING, // used when user types a malformed command or stdin is empty/contains just spaces
    UNKNOWN  // user types a command that does not exist
};

struct Vdb_Command {
    Vdb_Command_Type type;
    std::vector<Vdb_Token> args;
};

constexpr static size_t cmd_type_acc_tk_type_element_count = sizeof(cmd_type_acc_tk_type[0]) / sizeof(cmd_type_acc_tk_type[0][0]);

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
    const Vdb_Token next(const std::vector<Vdb_Token> &tokens, const Vdb_Token_Type *acc_types);
    Vdb_Command parse_command(const std::vector<Vdb_Token> &tokens);

    size_t pos = 0;
};
