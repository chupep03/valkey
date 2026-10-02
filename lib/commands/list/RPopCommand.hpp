#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// RPOP key [count]
// Removes and returns the last element(s) of a list.
// Without count: Bulk or nil. With count: Array (possibly empty)
// Wrong type -> WRONGTYPE
class RPopCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(1);

        std::string key(ctx.GetArgumentAsStr(0));
        const bool has_count = ctx.GetArgsCount() >= 2;

        long long count = 1;
        if (has_count) {
            count = ctx.GetArgumentAsLLInt(1);
            if (count < 0) {
                throw CommandException("value is out of range, must be positive");
            }
        }

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            if (has_count) ctx.Out().Array(std::vector<std::string>{});
            else ctx.Out().Nil();
            return;
        }

        auto* existing = std::get_if<Storage::ListType>(&entry->value);
        if (!existing) throw WrongTypeError();

        if (count == 0) {
            ctx.Out().Array(std::vector<std::string>{});
            return;
        }

        Storage::ListType list = *existing;

        const std::size_t take = std::min<std::size_t>(
            static_cast<std::size_t>(count), list.size());

        std::vector<std::string> popped;
        popped.reserve(take);
        for (std::size_t i = 0; i < take; i++) {
            popped.push_back(std::move(list.back()));
            list.pop_back();
        }

        if (list.empty()) {
            ctx.Storage().Remove(std::vector<std::string>{key});
        } else {
            ctx.Storage().Set(key, entry->WithValue(std::move(list)));
        }

        if (has_count) {
            ctx.Out().Array(popped);
        } else {
            ctx.Out().Bulk(popped.front());
        }
    }
};

} // namespace Commands