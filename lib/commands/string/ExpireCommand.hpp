#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <variant>

namespace Commands {
    

// EXPIRE key seconds
// Returns 1 if the TTL was set or the key was removed; 0 if no such key
// Negative or zero seconds delete the key    
class ExpireCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        long long seconds = ctx.GetArgumentAsLLInt(1);

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Int(0);
            return;
        }

        if (seconds <= 0) {
            ctx.Storage().Remove(std::vector<std::string>{key});
            ctx.Out().Int(1);
            return;
        }

        auto exp = std::chrono::system_clock::now() + std::chrono::seconds{seconds};
        Storage::Entry updated{entry->value, exp};
        ctx.Storage().Set(key, std::move(updated));
        ctx.Out().Int(1);
    }
}; 

} // namespace Command 
