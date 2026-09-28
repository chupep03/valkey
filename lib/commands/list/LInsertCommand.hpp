#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <algorithm>
#include <string>
#include <variant>

namespace Commands {

// LINSERT key BEFORE|AFTER pivot value
// Inserts value before or after the first occurrence of pivot
// If pivot is not found -> -1. If key is missing -> 0
// Otherwise -> new length. Wrong type -> WRONGTYPE
class LInsertCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(4);

        std::string key(ctx.GetArgumentAsStr(0));
        std::string where(ctx.GetArgumentAsStr(1));
        std::string pivot(ctx.GetArgumentAsStr(2));
        std::string value(ctx.GetArgumentAsStr(3));

        // Normalize BEFORE/AFTER
        for (char& c : where) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        const bool before = (where == "BEFORE");
        if (!before && where != "AFTER") throw CommandException(SyntaxError{}, "syntax error");

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Int(0);
            return;
        }

        auto* existing = std::get_if<Storage::ListType>(&entry->value);
        if (!existing) throw CommandException(WrongTypeError{});

        Storage::ListType list = *existing;
        auto it = std::find(list.begin(), list.end(), pivot);
        if (it == list.end()) {
            ctx.Out().Int(-1);
            return;
        }
        if (!before) ++it;
        list.insert(it, std::move(value));

        long long len = static_cast<long long>(list.size());
        ctx.Storage().Set(key, entry->WithValue(std::move(list)));
        ctx.Out().Int(len);
    }
};

} // namespace Commands