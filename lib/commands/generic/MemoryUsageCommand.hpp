#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <string>

namespace Commands {

// MEMORY USAGE key
// Returns an estimate of the memory used by key + value, in bytes
// Missing key -> nil. Replies with Int or nil
class MemoryUsageCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string key(ctx.GetArgumentAsStr(0));
        if (!ctx.Storage().Exist(key)) {
            ctx.Out().Nil();
            return;
        }
        ctx.Out().Int(static_cast<long long>(ctx.Storage().GetMemUsage(key)));
    }
};

} // namespace Commands