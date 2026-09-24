#include <string>
#include <stdexcept>

namespace Commands {

struct ArgsError {};
struct OtherError {};


class CommandException : public std::runtime_error {
private:
    using Msg = std::string;

public:
    CommandException(ArgsError, Msg msg) : std::runtime_error("ArgsError: " + msg) {}
};

} // namespace Commands
