#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"
#include "../../storage/Overloaded.hpp"

#include <string>
#include <variant>

namespace Commands {

// TYPE key
// Returns the type name: string, list, set, zset, hash, or none
// Missing or expired key -> "none"
// Replies with Bulk
class TypeCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(1);

        std::string key(ctx.GetArgumentAsStr(0));
        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Bulk("none");
            return;
        }

        std::string type = std::visit(Storage::Overloaded{
            [](const Storage::StringType&) { return std::string("string"); },
            [](const Storage::ListType&)   { return std::string("list");   },
            [](const Storage::SetType&)    { return std::string("set");    },
            [](const Storage::GeoType&)    { return std::string("zset");   }
        }, entry->value);

        ctx.Out().Bulk(type);
    }
};

} // namespace Commands