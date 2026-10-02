#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// SINTER key [key ...]
// Returns the intersection of the given sets. Missing keys count as empty
// Order is unspecified. Wrong type on any key -> WRONGTYPE
class SInterCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(1);
        std::vector<const Storage::SetType*> sets;
        sets.reserve(ctx.GetArgsCount());

        for (size_t i = 0; i < ctx.GetArgsCount(); i++) {
            std::string key(ctx.GetArgumentAsStr(i));
             auto entry = ctx.Storage().Get(key);
            if (!entry) {
                ctx.Out().Array({});
                return;
            }
            auto* existing = std::get_if<Storage::SetType>(&entry->value);
            if (!existing) throw WrongTypeError{};
            sets.push_back(existing);
        }

        auto smaller = [](const auto* a, const auto* b){ return a->size() < b->size(); };
        auto smallest_set = std::min_element(sets.begin(), sets.end(), smaller);

        Storage::SetType result = *(*smallest_set);
        for (const auto* s : sets) {
            if (s == *smallest_set) continue;
            for(auto it = result.begin(); it != result.end();) {
                if (!s->contains(*it)) {
                    it = result.erase(it);
                } else {
                    it++;
                }
            }
        }
        std::vector<std::string> out(result.begin(), result.end());
        ctx.Out().Array(out);
    }
};

} // namespace Commands