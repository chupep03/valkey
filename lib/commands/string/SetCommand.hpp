#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

namespace Commands {

// SET key value
// Replaces the entry, drops any existing TTL
class SetCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        std::string value(ctx.GetArgumentAsStr(1));

        ctx.Storage().Do(key,
            [&](std::shared_ptr<const Storage::Entry>) -> std::optional<Storage::Entry> {
                return Storage::Entry{Storage::StringType{std::move(value)}};
            });

        ctx.Out().Ok();
    }
};

} // namespace Commands