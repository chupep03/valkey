#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

namespace Commands {

// DBSIZE
// Returns the number of live keys (expired ones are purged first)
// Replies with Int
class DbSizeCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(0);
        ctx.Out().Int(static_cast<long long>(ctx.Storage().Size()));
    }
};

} // namespace Commands