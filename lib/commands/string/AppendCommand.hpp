#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <variant>

namespace Commands {
    
// APPEND key value
// Creates the key if missing. Preserves TTL of an existing key
// Returns the new length
class AppendCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        std::string value(ctx.GetArgumentAsStr(1));

        auto entry = ctx.Storage().Get(key);
        if(!entry) {
            // new key
            long long len = static_cast<long long>(value.length());
            Storage::Entry fresh{std::move(value)};
            ctx.Storage().Set(key, std::move(fresh));
            ctx.Out().Int(len);
            return;
        }

        auto* s = std::get_if<Storage::StringType>(&entry->value);
        if (!s) throw WrongTypeError();

        Storage::StringType combined = *s + value;
        long long len = static_cast<long long>(combined.size());
        // WithValue saves exp_time
        ctx.Storage().Set(key, entry->WithValue(std::move(combined)));
        ctx.Out().Int(len);
    } 
};

} // namespace Command 
