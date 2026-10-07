#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <variant>

namespace Commands {

// STRLEN key
// Missing key -> 0. Wrong type -> error
class StrlenCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string key(ctx.GetArgumentAsStr(0));

        auto entry = ctx.Storage().Get(key); // locked

        // for read commands when the storage_ shot is taken (Get method),
        // it makes no difference what happens to it after

        if (!entry) {
            ctx.Out().Int(0);
            return;
        }
        auto* s = std::get_if<Storage::StringType>(&entry->value);
        if (!s) throw WrongTypeError();

        ctx.Out().Int(static_cast<long long>(s->size()));
    }
};

} // namespace Commands