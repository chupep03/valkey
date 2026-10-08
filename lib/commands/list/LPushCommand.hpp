#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <variant>

namespace Commands {

// LPUSH key value [value...] 
// LPUSH k1 a b c
// One-by-time push
class LPushCommand : public ICommand {
public:
    void Execute(Context& ctx) {
        ctx.RequireMinArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            Storage::ListType list;
            for (size_t i = 0; i < ctx.GetArgsCount(); i++) {
                list.push_front(std::string(ctx.GetArgumentAsStr(i)));
            }
            
            long long len = static_cast<long long>(list.size());
            Storage::Entry fresh{std::move(list)};
            ctx.Storage().Set(key, std::move(fresh));
            ctx.Out().Int(len);
            return;
        }

        long long len = 0;

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
            Storage::ListType list = {};
            if (current) {
                auto* existing = std::get_if<Storage::ListType>(&current->value);
                if (!existing) throw WrongTypeError();
                list = *existing;
            }

            for (std::size_t i = 1; i < ctx.GetArgsCount(); i++) {
                list.push_front(std::string(ctx.GetArgumentAsStr(i)));
            }

            len = static_cast<long long>(list.size());
            if (current) return current->WithValue(std::move(list));
            return Storage::Entry{std::move(list)};
        });

        ctx.Out().Int(len);
    }
};

} // namespace Commands