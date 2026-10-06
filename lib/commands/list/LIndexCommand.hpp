#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <string>
#include <variant>

namespace Commands {

inline std::optional<std::size_t>
NormalizeIndex(long long raw, size_t n) {
    long long idx = raw;
    if (idx < 0) idx += static_cast<long long>(n);
    if (idx < 0 || idx >= static_cast<long long>(n)) return std::nullopt;
    return static_cast<size_t>(idx);
}


// LINDEX key index
// Returns the element at index. Negative indexes count from the end
// Out of range -> nil. Replies with Bulk or nil. Wrong type -> WRONGTYPE
class LIndexCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        long long index = ctx.GetArgumentAsLLInt(1);

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Nil();
            return;
        }

        auto* existing = std::get_if<Storage::ListType>(&entry->value);
        if (!existing) throw WrongTypeError();
        auto norm = NormalizeIndex(index, existing->size());
        if (!norm) {
            ctx.Out().Nil();
            return;
        }

        ctx.Out().Bulk((*existing)[*norm]);
    }
};

} // namespace Commands