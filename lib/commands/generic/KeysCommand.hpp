#pragma once

#include "../ICommand.hpp"
#include "../Context.hpp"

#include <string>
#include <vector>

namespace Commands {

// KEYS pattern
// Returns all keys matching a glob pattern
// Expired keys are not included. Replies with Array
class KeysCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string pattern(ctx.GetArgumentAsStr(0));
        auto keys = ctx.Storage().GetAllKeysByPattern(pattern);
        ctx.Out().Array(keys);
    }
};

} // namespace Commands