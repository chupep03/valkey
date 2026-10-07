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

        long long len = static_cast<long long>(value.length());
        Storage::StringType combined = value;

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
            if (!current) {
                return Storage::Entry{std::move(value)};
            } else {
                auto* s = std::get_if<Storage::StringType>(&current->value);
                if (!s) {
                    throw WrongTypeError();
                }
                combined = *s + value;
                len = static_cast<long long>(combined.size());
                return current->WithValue(std::move(combined));
            }
        });

        ctx.Out().Int(len);
    } 
};

} // namespace Commands
