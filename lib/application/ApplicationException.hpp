#pragma once

#include <stdexcept>
#include <string>

namespace Application {
    
class ApplicationException : public std::runtime_error {
public:
    explicit ApplicationException(const std::string& msg)
        : std::runtime_error("ApplicationException: " + msg) {}
};

class ParseException : public std::runtime_error {
public:
    explicit ParseException(const std::string& msg)
        : std::runtime_error("ParseException: " + msg) {}
};

} // namespace Application




