#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <cstddef>
#include <string>
#include <variant>

namespace Commands {

// SISMEMBER key member
// Checks whether member belongs to the set
// Replies with 1 if present, 0 otherwise. Wrong type -> WRONGTYPE
class SIsMemberCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        std::string member(ctx.GetArgumentAsStr(1));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Int(0);
            return;
        }
        auto* existing = std::get_if<Storage::SetType>(&entry->value);
        int count = 0;
        if (existing->count(member)) count = 1;
        ctx.Out().Int(count);
    }
};

} // namespace Commands