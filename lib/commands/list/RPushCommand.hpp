#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace Commands {

// RPUSH key value [value...]
// Appends values to the tail of the list. Creates the key if missing
// Preserves TTL of an existing key
// Returns new length. Wrong type -> WRONGTYPE
class RPushCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));

        long long len = 0;

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
                Storage::ListType list = {};
                if (current) {
                    auto* existing = std::get_if<Storage::ListType>(&current->value);
                    if (!existing) throw WrongTypeError();
                    list = *existing;
                }

                for (std::size_t i = 1; i < ctx.GetArgsCount(); i++) {
                    list.push_back(std::string(ctx.GetArgumentAsStr(i)));
                }

                len = static_cast<long long>(list.size());
                if (current) return current->WithValue(std::move(list));
                return Storage::Entry{std::move(list)};
            });

        ctx.Out().Int(len);
    }
};

} // namespace Commands