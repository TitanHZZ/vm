#pragma once
#include <string>
#include <vector>
#include <functional>

typedef enum {
    KEYWORD = 0,
    INSTRUCTION,
    INTEGER,
    FP,
    LABEL,
    DIRECTIVE,
    STRING,
    UNKNOWN
} Token_Type;

typedef struct {
    Token_Type type;
    std::string value;

    std::string file_path;
    size_t line_number;
    size_t line_offset;

    bool broken; // true when we know what the token is supposed to be but it's not well formatted
} Token;

class Lexer {
public:
    Lexer(const char *path, bool check_for_errors = true) : path(path), check_for_errors(check_for_errors) {}

    void print_tokens();
    static const char *type_as_cstr(Token_Type type);

    std::vector<Token> &tokenize();

private:
    std::string read_while(const std::function<bool(char)> &predicate);
    std::string read_string(bool &broken);
    void get_errors();

    const char *path;
    std::string line;
    size_t pos;

    std::vector<Token> tokens;

    bool check_for_errors;
};
