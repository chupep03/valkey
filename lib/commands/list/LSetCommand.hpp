#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace Commands {

// LSET key index value
// Overwrites the element at index. Negative indexes count from the end
// Missing key -> error. Index out of range -> IndexError
// Replies with OK. Wrong type -> WRONGTYPE
class LSetCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(3);
        std::string key(ctx.GetArgumentAsStr(0));
        long long index = ctx.GetArgumentAsLLInt(1);
        std::string value(ctx.GetArgumentAsStr(2));

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
                if (!current) throw CommandException("no such key");
                auto* existing = std::get_if<Storage::ListType>(&current->value);
                if (!existing) throw WrongTypeError();

                auto norm = NormalizeIndex(index, existing->size());
                if (!norm) throw IndexError("index out of range");

                Storage::ListType list = *existing;
                list[*norm] = std::move(value);
                return current->WithValue(std::move(list));
            });

        ctx.Out().Ok();
    }
};

} // namespace Commands