#pragma once

#include "../ICommand.hpp"
#include "../Context.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace Commands {

// DEL key [key ...]
// Removes one or more keys. Duplicates are counted once
// Replies with number of keys actually removed
class DelCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(1);
        std::vector<std::string> keys;
        keys.reserve(ctx.GetArgsCount());
        for (size_t i = 0; i < ctx.GetArgsCount(); i++) {
            keys.emplace_back(ctx.GetArgumentAsStr(i));
        }

        int removed = ctx.Storage().Remove(keys);
        ctx.Out().Int(removed);
    }
};

} // namespace Commands