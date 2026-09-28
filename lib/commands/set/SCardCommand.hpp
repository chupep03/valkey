#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <string>
#include <variant>

namespace Commands {

// SCARD key
// Returns the number of members in a set. Missing key -> 0
// Wrong type -> WRONGTYPE
class SCardCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string key(ctx.GetArgumentAsStr(0));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Int(0);
            return;
        }

        auto* existing = std::get_if<Storage::SetType>(&entry->value);
        if (!existing) throw CommandException(WrongTypeError{});
        ctx.Out().Int(static_cast<long long>(existing->size()));
    }
};

} // namespace Commands