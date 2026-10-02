#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <string>
#include <variant>

namespace Commands {

// LSET key index value
// Overwrites the element at index. Negative indexes count from the end
// Index out of range -> error IndexError
// Replies with OK. Wrong type -> WRONGTYPE
class LSetCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(3);
        std::string key(ctx.GetArgumentAsStr(0));
        long long index = ctx.GetArgumentAsLLInt(1);
        std::string value(ctx.GetArgumentAsStr(2));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            throw CommandException("no such key");
        }

        auto* existing = std::get_if<Storage::ListType>(&entry->value);
        if (!existing) throw WrongTypeError();
        auto norm = NormalizeIndex(index, existing->size());
        if (!norm) {
            throw IndexError("index out of range");
        }

        Storage::ListType list = *existing;
        list[*norm] = std::move(value);
        ctx.Storage().Set(key, entry->WithValue(std::move(list)));
        ctx.Out().Ok();
    }
};

} // namespace Commands