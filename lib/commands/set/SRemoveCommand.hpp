#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <cstddef>
#include <string>
#include <variant>

namespace Commands {

// SREM key member [member ...]
// Removes one or more members from a set
// Missing members are ignored and not counted
// Replies with Int(number of removed). Wrong type -> WRONGTYPE
class SRemoveCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));

        long long removed = 0;

        ctx.Storage().Do(key,
            [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
                if (!current) {
                    removed = 0;
                    return std::nullopt;
                }

                auto* existing = std::get_if<Storage::SetType>(&current->value);
                if (!existing) throw WrongTypeError();
                Storage::SetType set = *existing;

                for (std::size_t i = 1; i < ctx.GetArgsCount(); ++i) {
                    removed += static_cast<long long>(
                        set.erase(std::string(ctx.GetArgumentAsStr(i))));
                }

                if (set.empty()) return std::nullopt; 
                return current->WithValue(std::move(set));
            });

        ctx.Out().Int(removed);
    }
};

} // namespace Commands