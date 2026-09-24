#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <chrono>

namespace Commands {

// TTL key
//   -2 : key does not exist
//   -1 : key exists but has no TTL
//   >=0: seconds remaining
class TTLCommnd : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);
        std::string key(ctx.GetArgumentAsStr(0));

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Int(-2);
            return;
        }

        if (!entry->exp_time.has_value()) {
            ctx.Out().Int(-1);
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(entry->exp_time.value() - now).count();
        if (remaining < 0){
            remaining = 0;
        } 

        ctx.Out().Int(static_cast<long long>(remaining));
    }
};

}// namespace Commands