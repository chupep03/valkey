#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <string>
#include <variant>
#include <vector>

namespace Commands {

// SMEMBERS key
// Returns all members of a set. Order is unspecified
// Missing key -> empty Array. Wrong type -> WRONGTYPE
class SMembersCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string key(ctx.GetArgumentAsStr(0));
        
        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Array({});
            return;
        }

        auto* existing = std::get_if<Storage::SetType>(&entry->value);
        if (!existing) throw WrongTypeError();
        std::vector<std::string> result(existing->begin(), existing->end());
        ctx.Out().Array(result);
    }
};

} // namespace Commands
