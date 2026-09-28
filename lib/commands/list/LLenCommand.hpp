#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <string>
#include <variant>

namespace Commands {

// LLEN key
// Returns the length of a list. Missing key -> 0. Wrong type -> WRONGTYPE
// Always replies with Int.
class LLenCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);

        std::string key(ctx.GetArgumentAsStr(0));
        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Int(0);
            return;
        }

        auto* existing = std::get_if<Storage::ListType>(&entry->value);
        if (!existing) throw CommandException(WrongTypeError{});

        ctx.Out().Int(static_cast<long long>(existing->size()));
    }
};

} // namespace Commands