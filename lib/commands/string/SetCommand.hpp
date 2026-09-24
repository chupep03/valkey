#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

namespace Commands {

// SET key value
// Replaces the entry, drops any existing TTL
class SetCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        std::string value(ctx.GetArgumentAsStr(1));

        Storage::Entry entry{Storage::StringType{std::move(value)}};
        ctx.GetStorage().Set(key, std::move(entry));
        ctx.GetOut().Ok();
    }
};

} // namespace Commands