#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// SDIFF key [key ...]
// Returns elements of the first set that are not in any of the others
// Missing keys count as empty sets. Order is unspecified
// Wrong type on any key -> WRONGTYPE
class SDiffCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(1);
        std::string first_key(ctx.GetArgumentAsStr(0));
        auto first_entry = ctx.Storage().Get(first_key);

        if (!first_entry) {
            ctx.Out().Array({});
            return;
        }
        auto* first = std::get_if<Storage::SetType>(&first_entry->value);
        if (!first) throw WrongTypeError();
        Storage::SetType result = *first;

        for (std::size_t i = 1; i < ctx.GetArgsCount(); ++i) {
            std::string key(ctx.GetArgumentAsStr(i));
            auto entry = ctx.Storage().Get(key);
            if (!entry) continue;
            auto* existing = std::get_if<Storage::SetType>(&entry->value);
            if (!existing) throw WrongTypeError();

            for (const auto& item : *existing) result.erase(item);
            if (result.empty()) break;
        }

        std::vector<std::string> out(result.begin(), result.end());
        ctx.Out().Array(out);
    }
};

} // namespace Commands