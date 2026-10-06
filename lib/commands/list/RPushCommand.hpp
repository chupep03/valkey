#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <variant>

namespace Commands {

// LPUSH key value [value...] 
// LPUSH k1 a b c
// One-by-time push
class RPushCommand : public ICommand {
public:
    void Execute(Context& ctx) {
        ctx.RequireMinArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            Storage::ListType list;
            for (size_t i = 1; i < ctx.GetArgsCount(); i++) {
                list.push_back(std::string(ctx.GetArgumentAsStr(i)));
            }
            
            long long len = static_cast<long long>(list.size());
            Storage::Entry fresh{std::move(list)};
            ctx.Storage().Set(key, std::move(fresh));
            ctx.Out().Int(len);
            return;
        }
        auto* existing = std::get_if<Storage::ListType>(&entry->value);
        if (!existing) throw WrongTypeError();

        Storage::ListType list = *existing;
        for (std::size_t i = 1; i < ctx.GetArgsCount(); i++) {
            list.push_back(std::string(ctx.GetArgumentAsStr(i)));
            //std::cout << std::string(ctx.GetArgumentAsStr(i)) << "\n";
        }

        long long len = static_cast<long long>(list.size());
        ctx.Storage().Set(key, entry->WithValue(std::move(list)));
        ctx.Out().Int(len);
    }
};

} // namespace Commands