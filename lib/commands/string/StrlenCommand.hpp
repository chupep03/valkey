#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <variant>

namespace Commands {

// STRLEN key
// Missing key -> 0. Wrong type -> error
class StrlenCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string key(ctx.GetArgumentAsStr(0));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Int(0);
            return;
        }
        auto* s = std::get_if<Storage::StringType>(&entry->value);
        if (!s) throw CommandException(WrongTypeError{});

        ctx.Out().Int(static_cast<long long>(s->size()));
    }
};

} // namespace Commands