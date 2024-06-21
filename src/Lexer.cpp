#include <iostream>
#include <vector>
#include <fstream>
#include <functional>
#include <string>
#include <iomanip>

// the order of this headers is mandatory as something from the Windows API (from cout_colors.h) is conflicting with our names
#include "inst.h"
#include "cout_colors.h"

typedef enum {
    KEYWORD, INSTRUCTION, NUMBER, LABEL, DIRECTIVE, STRING, UNKNOWN
} Token_Type;

typedef struct {
    Token_Type type;
    std::string value;

    size_t line_number;
    size_t line_offset;

    bool broken; // true when we know what the token is supposed to be but it's not well formatted
} Token;

class Lexer {
public:
    Lexer(const char *path) : path(path) {}

    int print_errors() {
        int e = 0;

        for (Token &token: tokens) {
            if (token.type == Token_Type::UNKNOWN) {
                setConsoleColor(RED); std::cerr << "ERROR: "; resetConsoleColor();
                setConsoleColor(YELLOW); std::cerr << path << ":" << token.line_number << ":" << token.line_offset; resetConsoleColor();
                std::cerr << ": Unrecognizable token ";
                setConsoleColor(BG_BLUE); std::cout << token.value; resetConsoleColor();
                std::cout << "." << std::endl;
                e++;
            } else if (token.broken) {
                setConsoleColor(RED); std::cerr << "ERROR: "; resetConsoleColor();
                setConsoleColor(YELLOW); std::cerr << path << ":" << token.line_number << ":" << token.line_offset; resetConsoleColor();
                std::cerr << ": Malformed token ";
                setConsoleColor(BG_BLUE); std::cout << token.value; resetConsoleColor();
                std::cout << " of type " << type_as_cstr(token.type) << "." << std::endl;
                e++;
            }
        }

        return e;
    }

    void print_tokens() {
        for (Token &token: tokens) {
            setConsoleColor(YELLOW); std::cout << std::setw(13) << std::left << type_as_cstr(token.type); resetConsoleColor();
            std::cout << "\t" << token.value << std::endl;
        }
    }

    std::vector<Token> tokenize () {
        // open the file
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "ERROR: An error occured when while trying to read source code from '" << path << "'." << std::endl;
            exit(1);
        }

        size_t pos_start = 1;
        size_t line_number = 1;
        while (std::getline(file, line)) {
            pos = 0;
            while (pos < line.size()) {
                pos_start = pos + 1;

                if (std::isspace(line[pos])) {
                    // handle spaces
                    pos++;

                } else if (line[pos] == '#') {
                    // handle comments
                    pos = line.size();

                } else if (line[pos] == '%') {
                    // handle preprocessor directives
                    pos++; // ignore the '%'
                    const std::string directive = read_while([](char c) { return !std::isspace(c); });
                    tokens.push_back({Token_Type::DIRECTIVE, std::move(directive), line_number, pos_start, false});

                } else if (line[pos] == '"') {
                    // handle string literals
                    bool broken = false;
                    const std::string str = read_string(broken);
                    tokens.push_back({Token_Type::STRING, std::move(str), line_number, pos_start, broken});

                } else if (std::isdigit(line[pos]) || line[pos] == '-') {
                    // handle numbers
                    const std::string number = read_while([](char c) { return (c >= 48 && c <= 57) || c == '.' || c == ',' || c == '-' || c == 'e'; });
                    tokens.push_back({Token_Type::NUMBER, std::move(number), line_number, pos_start, false});

                } else if (std::isalpha(line[pos]) || line[pos] == '_') {
                    // handle instructions and labels
                    std::string word = read_while([](char c) { return std::isalnum(c) || c == '_' || c == ':'; });
                    if (word.back() == ':') {
                        word.pop_back();
                        tokens.push_back({LABEL, std::move(word), line_number, pos_start, false});
                    } else if (str_is_inst(word)) {
                        tokens.push_back({INSTRUCTION, std::move(word), line_number, pos_start, false});
                    } else {
                        tokens.push_back({KEYWORD, std::move(word), line_number, pos_start, false});
                    }

                } else {
                    // handle unknown tokens
                    const std::string val = read_while([](char c) { return !std::isspace(c); });
                    tokens.push_back({UNKNOWN, std::move(val), line_number, pos_start, false});
                    pos++;
                }
            }

            line_number++;
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

    std::string read_string(bool &broken) {
        const size_t start = pos;
        pos++; // ignore the '"'
        std::string str; // result string

        while (pos < line.size() && line[pos] != '"') {
            if (line[pos] == '\\') {
                // handle escaped chars
                pos++;
                switch (line[pos]) {
                case 'n':
                    str += '\n';
                    break;
                
                case 't':
                    str += '\t';
                    break;

                case 'r':
                    str += '\r';
                    break;

                case '\\':
                    str += '\\';
                    break;

                case '"':
                    str += '"';
                    break;

                default:
                    str += line[pos];
                    break;
                }

                if (pos == line.size() - 1) {
                    // handle unclosed string with escaping char at the end
                    str = line.substr(start, line.size() - 1);
                    broken = true;
                }
            } else if (pos == line.size() - 1) {
                // handle unclosed string
                str += line[pos];
                str.insert(0, "\"");
                broken = true;
            } else {
                str += line[pos];
            }

            pos++;
        }

        pos++;
        return str;
    }

    const char *type_as_cstr(Token_Type type) {
        switch (type) {
        case Token_Type::KEYWORD:
            return "KEYWORD";

        case Token_Type::INSTRUCTION:
            return "INSTRUCTION";

        case Token_Type::NUMBER:
            return "NUMBER";

        case Token_Type::LABEL:
            return "LABEL";

        case Token_Type::DIRECTIVE:
            return "DIRECTIVE";

        case Token_Type::STRING:
            return "STRING";

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
    Lexer lexer("./examples/hello_world.vasm");
    lexer.tokenize();

    if (lexer.print_errors() != 0)
        return 1;

    lexer.print_tokens();

    return 0;
}
