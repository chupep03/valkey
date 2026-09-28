#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

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
        auto entry = ctx.Storage().Get(key);
        long long added = 0;

        if (!entry) {
            Storage::SetType set;
            for (std::size_t i = 1; i < ctx.GetArgsCount(); i++) {
                auto [_, inserted] = set.insert(std::string(ctx.GetArgumentAsStr(i)));
                if (inserted) added++;
            }
            Storage::Entry fresh{std::move(set)};
            ctx.Storage().Set(key, std::move(fresh));
            ctx.Out().Int(added);
            return;
        }

        auto* existing = std::get_if<Storage::SetType>(&entry->value);
        if (!existing) throw CommandException(WrongTypeError{});

        Storage::SetType set = *existing;
        for (std::size_t i = 1; i < ctx.GetArgsCount(); ++i) {
            auto [_, inserted] = set.insert(std::string(ctx.GetArgumentAsStr(i)));
            if (inserted) added++;
        }

        ctx.Storage().Set(key, entry->WithValue(std::move(set)));
        ctx.Out().Int(added);
    }
};

} // namespace Commands