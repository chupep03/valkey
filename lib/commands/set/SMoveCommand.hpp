#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <string>
#include <variant>
#include <vector>

namespace Commands {

// SMOVE source destination member
// Moves member from source set to destination set
// If member is not in source -> 0, nothing changes
// Otherwise -> 1. Wrong type on either key -> WRONGTYPE
class SMoveCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(3);
        std::string src_key(ctx.GetArgumentAsStr(0));
        std::string dst_key(ctx.GetArgumentAsStr(1));
        std::string member(ctx.GetArgumentAsStr(2));

        auto dst_entry = ctx.Storage().Get(dst_key);
        if (dst_entry) {
            auto* d = std::get_if<Storage::SetType>(&dst_entry->value);
            if (!d) throw CommandException(WrongTypeError{});
        }

        auto src_entry = ctx.Storage().Get(src_key);
        if (!src_entry) {
            ctx.Out().Int(0);
            return;
        }
        auto* src = std::get_if<Storage::SetType>(&src_entry->value);
        if (!src) throw CommandException(WrongTypeError{});

        if (src->count(member) == 0) {
            ctx.Out().Int(0);
            return;
        }

        Storage::SetType new_src = *src;
        new_src.erase(member);
        Storage::SetType new_dst;
        if (dst_entry) {
            new_dst = std::get<Storage::SetType>(dst_entry->value);
        }
        new_dst.insert(member);

        if (new_src.empty()) {
            ctx.Storage().Remove(std::vector<std::string>{src_key});
        } else {
            ctx.Storage().Set(src_key, src_entry->WithValue(std::move(new_src)));
        }
        ctx.Storage().Set(dst_key, Storage::Entry{std::move(new_dst)});

        ctx.Out().Int(1);
    }
};

} // namespace Commands