#pragma once

#include <string>
#include <stdexcept>

namespace Commands {

//struct ArgsError {};
//struct WrongTypeError {};
//struct IndexError {};
//struct SyntaxError {};

class CommandException : public std::runtime_error {
public:
    explicit CommandException(const std::string& msg = "no details") 
        : std::runtime_error("CommandException: " + msg) {}
};

class ArgsError : public std::runtime_error {
public:     
    explicit ArgsError(const std::string& msg = "no details")
        : std::runtime_error("ArgsError: " + msg) {}
};

class WrongTypeError : public std::runtime_error {
public:     
    explicit WrongTypeError(const std::string& msg = "no details")
        : std::runtime_error("WrongTypeError: " + msg) {}
};

class IndexError : public std::runtime_error {
public:
    explicit IndexError(const std::string& msg = "no details")
        : std::runtime_error("IndexError: " + msg) {}
};

class SyntaxError : public std::runtime_error {
public:
    explicit SyntaxError(const std::string& msg = "no details")
        : std::runtime_error("SyntaxError: " + msg) {}
};

} // namespace Commands
