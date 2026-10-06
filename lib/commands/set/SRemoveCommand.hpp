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
// Replies with Int(number removed). Wrong type -> WRONGTYPE
class SRemoveCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        auto entry = ctx.Storage().Get(key);
        size_t removed = 0;

        if (!entry) {
            ctx.Out().Int(0);
            return;
        }
        auto* existing = std::get_if<Storage::SetType>(&entry->value);
        if (!existing) WrongTypeError();
        Storage::SetType set = *existing;

        for (size_t i = 0; i < ctx.GetArgsCount(); i++)
            removed += static_cast<long long>(set.erase(std::string(ctx.GetArgumentAsStr(i))));
        
        if (set.empty()) {
            ctx.Storage().Remove(std::vector<std::string>{key});
        } else {
            ctx.Storage().Set(key, entry->WithValue(std::move(set)));
        }
        ctx.Out().Int(removed);
    }
};

} // namespace Commands