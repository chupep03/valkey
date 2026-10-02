#pragma once

#include <string>
#include <vector>

namespace Parser {

class Tokenizer {
public:
    std::vector<std::string> operator()(const std::string& line) const {
        std::vector<std::string> tokens;
        std::string current;
        bool in_token = false; 

        for (char ch : line) {
            switch (ch) {
    
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case '\v':
            case '\f':
                if (in_token) {
                    tokens.push_back(std::move(current));
                    current.clear();
                    in_token = false;
                }
                break;

            default:
                current.push_back(ch);
                in_token = true;
                break;
            }
        }

        if (in_token) {
            tokens.push_back(std::move(current));
        }

        return tokens;
    }
};

} // namespace Parser