#pragma once

#include <string>
#include <stdexcept>

namespace Commands {

class CommandException : public std::runtime_error {
public:
    explicit CommandException(const std::string& msg = "no details")
        : std::runtime_error(msg) {}
};

class ArgsError : public CommandException {
public:
    explicit ArgsError(const std::string& msg = "no details")
        : CommandException("ArgsError: " + msg) {}
};

class WrongTypeError : public CommandException {
public:
    explicit WrongTypeError(const std::string& msg = "no details")
        : CommandException("WrongTypeError: " + msg) {}
};

class IndexError : public CommandException {
public:
    explicit IndexError(const std::string& msg = "no details")
        : CommandException("IndexError: " + msg) {}
};

class SyntaxError : public CommandException {
public:
    explicit SyntaxError(const std::string& msg = "no details")
        : CommandException("SyntaxError: " + msg) {}
};

} // namespace Commands