#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <variant>

namespace Commands {

// GET key
class GetCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string key(ctx.GetArgumentAsStr(0));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Nil();
            return;
        }
        auto* s = std::get_if<Storage::StringType>(&entry->value);
        if (!s) throw CommandException(WrongTypeError{});

        ctx.Out().Bulk(*s);
    }
};

} // namespace Commands