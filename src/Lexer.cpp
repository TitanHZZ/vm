#include <iostream>
#include <vector>
#include <fstream>
#include <functional>

#include "cout_colors.h"

enum Token_Type {
    KEYWORD, NUMBER, LABEL, DIRECTIVE, UNKNOWN
};

struct Token {
    Token_Type type;
    std::string value;

    size_t line_number;
    size_t line_offset;
};

class Lexer {
public:
    Lexer(const char *path) : path(path) {}

    int print_errors() {
        int e = 0;

        for (Token &token: tokens) {
            if (token.type == Token_Type::UNKNOWN) {
                setConsoleColor(RED); std::cerr << "ERROR: "; resetConsoleColor();
                setConsoleColor(YELLOW); std::cerr << path << ":" << token.line_number << ":" << token.line_offset; resetConsoleColor();
                std::cerr << ": Unknown token \"" << token.value << "\"." << std::endl;
                e++;
            }
        }

        return e;
    }

    void print_tokens() {
        for (Token &token: tokens) {
            std::cout << type_as_cstr(token.type) << "\t" << token.value << std::endl;
        }
    }

    std::vector<Token> tokenize () {
        // open the file
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "ERROR: An error occured when while trying to read source code from '" << path << "'." << std::endl;
            exit(1);
        }

        size_t pos_start = pos;
        size_t line_number = 1;
        while (std::getline(file, line)) {
            pos = 0;
            while (pos < line.size()) {
                if (std::isspace(line[pos])) {
                    // handle spaces
                    pos++;

                } else if (line[pos] == '#') {
                    // handle comments
                    pos = line.size();

                } else if (line[pos] == '%') {
                    // handle preprocessor directives
                    pos++;
                    const std::string directive = read_while([](char c) { return !std::isspace(c); });
                    tokens.push_back({Token_Type::DIRECTIVE, std::move(directive), line_number, pos_start});

                } else if (std::isdigit(line[pos]) || line[pos] == '-') {
                    // handle numbers
                    const std::string number = read_while([](char c) { return (c >= 48 && c <= 57) || c == '.' || c == ',' || c == '-' || c == 'e'; });
                    tokens.push_back({Token_Type::NUMBER, std::move(number), line_number, pos_start});

                } else if (std::isalpha(line[pos])) {
                    // handle instructions and labels
                    const std::string word = read_while([](char c) { return std::isalnum(c) || c == ':'; });
                    if (word.back() == ':') {
                        tokens.push_back({LABEL, std::move(word), line_number, pos_start});
                    } else {
                        tokens.push_back({KEYWORD, std::move(word), line_number, pos_start});
                    }

                } else {
                    // handle unknown characters
                    const std::string val = read_while([](char c) { return !std::isspace(c); });
                    tokens.push_back({UNKNOWN, std::move(val), line_number, pos_start});
                    pos++;
                }
            }

            line_number++;
            pos_start = pos;
        }

        return tokens;
    }

private:
    std::string read_while(const std::function<bool(char)> &predicate) {
        size_t start = pos;
        while (pos < line.size() && predicate(line[pos])) {
            pos++;
        }
        return line.substr(start, pos - start);
    }

    const char *type_as_cstr(Token_Type type) {
        switch (type) {
        case Token_Type::KEYWORD:
            return "KEYWORD  ";

        case Token_Type::NUMBER:
            return "NUMBER   ";

        case Token_Type::LABEL:
            return "LABEL    ";

        case Token_Type::DIRECTIVE:
            return "DIRECTIVE";

        case Token_Type::UNKNOWN:
        default:
            std::cerr << "ERROR: Unknown token type." << std::endl;
            exit(1);
        }
    }

    const char *path;
    std::string line;
    size_t pos;

    std::vector<Token> tokens;
};

int main() {
    Lexer lexer("./examples/e.vasm");
    lexer.tokenize();
    lexer.print_errors();
    lexer.print_tokens();

    return 0;
}
