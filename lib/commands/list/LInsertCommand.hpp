#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <algorithm>
#include <string>
#include <variant>

namespace Commands {

// LINSERT key BEFORE|AFTER pivot value
// Inserts value before or after the first pivot
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
        if (!before && where != "AFTER") throw SyntaxError("syntax error");

        int result = 0;

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
            if (!current) {
                result = 0;
                return std::nullopt;
            }
            auto* existing = std::get_if<Storage::ListType>(&current->value);
            if (!existing) throw WrongTypeError();
            Storage::ListType list = *existing;
            auto it = std::find(list.begin(), list.end(), pivot);
            if (it == list.end()) {
                result = -1;
                return current->WithValue(std::move(list));
            }
            if (!before) it++;
            list.insert(it, std::move(value));
            result = static_cast<long long>(list.size());
            return current->WithValue(std::move(list));
        });
        ctx.Out().Int(result);
    }
};

} // namespace Commands