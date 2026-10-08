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

        int result = 0;
        auto exp = std::chrono::system_clock::now() + std::chrono::seconds{seconds};

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
            if (!current) {
                result = 0;
                return std::nullopt;
            } 
            if (seconds <= 0) {
                result = 1;
                return std::nullopt;
            } 
            result = 1;
            return Storage::Entry{current->value, exp};
        });

        ctx.Out().Int(result);
    }
}; 

} // namespace Commands
