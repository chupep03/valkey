#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <cstddef>
#include <string>
#include <variant>

namespace Commands {

// SADD key member [member ...]
// Adds one or more members to a set. Creates the key if missing
// Duplicates are ignored and not counted
// Replies with Int(number of new members). Wrong type -> WRONGTYPE
class SAddCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        size_t added = 0;

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
            Storage::SetType set;
            if (current) {
                auto* existing = std::get_if<Storage::SetType>(&current->value);
                if (!existing) throw WrongTypeError();
                set = *existing;
            }

            for (std::size_t i = 1; i < ctx.GetArgsCount(); i++) {
                auto [_, inserted] = set.insert(std::string(ctx.GetArgumentAsStr(i)));
                if (inserted) added++;
            }
            if (current) return current->WithValue(std::move(set));
            return Storage::Entry{std::move(set)};
        });
        ctx.Out().Int(added);
    }
};

} // namespace Commands