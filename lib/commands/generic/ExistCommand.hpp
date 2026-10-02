#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <cstddef>
#include <string>

namespace Commands {

// EXISTS key [key ...]
// Checks how many of the given keys exist
// Duplicates are counted multiple times (EXISTS a a -> 2 if a exists)
// Replies with Int
class ExistsCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(1);
        long long count = 0;
        for (std::size_t i = 0; i < ctx.GetArgsCount(); i++) {
            std::string key(ctx.GetArgumentAsStr(i));
            if (ctx.Storage().Exist(key)) count++;
        }
        ctx.Out().Int(count);
    }
};

} // namespace Commands