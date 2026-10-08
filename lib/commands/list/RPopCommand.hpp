#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// RPOP key [count]
// Removes and returns the last n elements of a list
// Without count: bulk or nil. With count: Array
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
            if (count < 0)
                throw CommandException("value is out of range, must be positive");
        }

        if (count == 0) {
            auto entry = ctx.Storage().Get(key);
            if (entry) {
                if (!std::get_if<Storage::ListType>(&entry->value))
                    throw WrongTypeError();
            }
            ctx.Out().Array(std::vector<std::string>{});
            return;
        }

        std::vector<std::string> popped;
        bool key_existed = false;

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
                if (!current) return std::nullopt;
                auto* existing = std::get_if<Storage::ListType>(&current->value);
                if (!existing) throw WrongTypeError();

                key_existed = true;

                Storage::ListType list = *existing;
                size_t take = std::min<size_t>(static_cast<size_t>(count), list.size());
                popped.reserve(take);
                for (std::size_t i = 0; i < take; i++) {
                    popped.push_back(std::move(list.back()));
                    list.pop_back();
                }

                if (list.empty()) return std::nullopt;  
                return current->WithValue(std::move(list));
            });

        if (!key_existed) {
            if (has_count) ctx.Out().Array(std::vector<std::string>{});
            else ctx.Out().Nil();
            return;
        }

        if (has_count) ctx.Out().Array(popped);
        else ctx.Out().Bulk(popped.front());
    }
};

} // namespace Commands