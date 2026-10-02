#pragma once

#include "Context.hpp"

namespace Commands {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void Execute(Context& ctx) = 0;
};

} // namespace Commands