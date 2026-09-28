#pragma once

#include <string>
#include <stdexcept>

namespace Commands {

struct ArgsError {};
struct WrongTypeError {};
struct IndexError {};
struct SyntaxError {};
struct OtherError {};

const std::string def = "no exception details"; 

class CommandException : public std::runtime_error {
private:
    using Msg = std::string;

public:
    explicit CommandException(ArgsError, Msg msg = def) : std::runtime_error("ArgsError: " + msg) {}
    explicit CommandException(WrongTypeError, Msg msg = def) : std::runtime_error("WrongTypeError: " + msg) {}
    explicit CommandException(IndexError, Msg msg = def) : std::runtime_error("IndexError: " + msg) {}
    explicit CommandException(SyntaxError, Msg msg = def) : std::runtime_error("IndexError: " + msg) {}
    explicit CommandException(OtherError, Msg msg = def) : std::runtime_error("OtherError: " + msg) {}
};

} // namespace Commands
