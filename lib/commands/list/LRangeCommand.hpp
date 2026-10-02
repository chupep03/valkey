#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp" 

#include <algorithm>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Commands {


// LRANGE key start stop
// Returns the sub-list between indexes start and stop
// Negative indexes count from the end (-1 = last)
// Replies with Array. Wrong type -> WRONGTYPE
class LRangeCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(3);
        std::string key(ctx.GetArgumentAsStr(0));
        long long start = ctx.GetArgumentAsLLInt(1);
        long long stop  = ctx.GetArgumentAsLLInt(2);

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Array({});
            return;
        }

        auto* existing = std::get_if<Storage::ListType>(&entry->value);
        if (!existing) throw WrongTypeError();

        const auto n = static_cast<long long>(existing->size());

        // Redis index standart
        if (start < 0) start += n;
        if (stop  < 0) stop  += n;
        if (start < 0) start = 0;
        if (stop >= n) stop  = n - 1;
        if (start > stop || start >= n) {
            ctx.Out().Array({});
            return;
        }

        std::vector<std::string> result;
        result.reserve(static_cast<std::size_t>(stop - start + 1));
        for (long long i = start; i <= stop; ++i) {
            result.push_back((*existing)[static_cast<std::size_t>(i)]);
        }
        ctx.Out().Array(result);
    }
};

} // namespace Commands