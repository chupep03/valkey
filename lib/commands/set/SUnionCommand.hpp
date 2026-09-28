#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// SUNION key [key ...]
// Returns the union of the given sets. Missing keys count as empty sets
// Order is unspecified. Wrong type on any key -> WRONGTYPE
class SUnionCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(1);

        Storage::SetType result;

        for (std::size_t i = 0; i < ctx.GetArgsCount(); ++i) {
            std::string key(ctx.GetArgumentAsStr(i));
            auto entry = ctx.Storage().Get(key);
            if (!entry) continue;
            auto* existing = std::get_if<Storage::SetType>(&entry->value);
            if (!existing) throw CommandException(WrongTypeError{});
            result.insert(existing->begin(), existing->end());
        }

        std::vector<std::string> out(result.begin(), result.end());
        ctx.Out().Array(out);
    }
};

} // namespace Commands