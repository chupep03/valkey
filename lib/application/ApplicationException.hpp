#pragma once

#include <stdexcept>
#include <string>

namespace Application {
    
class ApplicationException : public std::runtime_error {
public:
    explicit ApplicationException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class ParseException : public ApplicationException{
public:
    explicit ParseException(const std::string& msg)
        : ApplicationException("ApplicationException: " + msg) {}
};

} // namespace Application




