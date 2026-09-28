#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

namespace Commands {

// FLUSHDB
// Removes all keys from the database and resets memory accounting.
// Replies with OK.
class FlushDbCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(0);
        ctx.Storage().Flush();
        ctx.Out().Ok();
    }
};

} // namespace Commands