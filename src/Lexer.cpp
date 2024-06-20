#include <iostream>
#include <vector>
#include <fstream>
#include <functional>

enum Token_Type {
    KEYWORD, NUMBER, LABEL, UNKNOWN
};

const char *type_as_cstr(Token_Type type) {
    switch (type) {
    case Token_Type::KEYWORD:
        return "KEYWORD";
    
    case Token_Type::NUMBER:
        return "NUMBER";
    
    case Token_Type::LABEL:
        return "LABEL";
    
    case Token_Type::UNKNOWN:
    default:
        std::cerr << "ERROR: unknown token type" << std::endl;
        exit(1);
    }
}

struct Token {
    Token_Type type;
    std::string value;
};

class Lexer {
public:
    Lexer(const char *path) : path(path) {}

    std::vector<Token> tokenize () {
        // open the file
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "ERROR: An error occured when while trying to read source code from '" << path << "'." << std::endl;
            exit(1);
        }

        std::vector<Token> tokens;
        while (std::getline(file, line)) {
            pos = 0;
            while (pos < line.size()) {
                if (std::isspace(line[pos])) {
                    // handle spaces
                    pos++;
                } else if (line[pos] == '#') {
                    // handle comments
                    pos = line.size();
                } else if (std::isdigit(line[pos])) {
                    // handle numbers
                    const std::string number = read_while([](char c) { return (c >= 48 && c <= 57) || c == '.' || c == ',' || c == '-' || c == 'e'; });
                    tokens.push_back({Token_Type::NUMBER, std::move(number)});
                } else if (std::isalpha(line[pos])) {
                    // handle instructions and labels
                    const std::string word = read_while([](char c) { return std::isalnum(c) || c == ':'; });
                    if (word.back() == ':') {
                        tokens.push_back({LABEL, std::move(word)});
                    } else {
                        tokens.push_back({KEYWORD, std::move(word)});
                    }
                } else {
                    // handle unknown characters
                    tokens.push_back({UNKNOWN, std::string(1, line[pos])});
                    pos++;
                }
            }
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

    const char *path;
    std::string line;
    size_t pos;
};

int main() {

    Lexer lexer("./examples/123i.vasm");
    std::vector<Token> tokens = lexer.tokenize();

    for (Token &t: tokens) {
        std::cout << type_as_cstr(t.type) << "\t\t" << t.value << std::endl;
    }

    return 0;
}
