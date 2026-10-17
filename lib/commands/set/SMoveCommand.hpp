#pragma once

#include "commands/Context.hpp"
#include "storage/StorageEngine.hpp"

#include <array>
#include <string>
#include <variant>

namespace Commands {

// SMOVE source destination member
// Moves member from source set to destination set
// If member is not in source -> int(0)
// Otherwise -> int(1). Wrong type on either key -> WRONGTYPE
class SMoveCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(3);

        std::string src_key(ctx.GetArgumentAsStr(0));
        std::string dst_key(ctx.GetArgumentAsStr(1));
        std::string member(ctx.GetArgumentAsStr(2));

        if (src_key == dst_key) {
            auto cur = ctx.Storage().Get(src_key);
            if (!cur) { ctx.Out().Int(0); return; }
            auto* s = std::get_if<Storage::SetType>(&cur->GetValue());
            if (!s) throw WrongTypeError{};
            ctx.Out().Int(s->count(member) ? 1 : 0);
            return;
        }

        bool moved = false;

        ctx.Storage().DoMulti([&](Storage::EntryPtrArray<2> cur) -> std::array<Storage::KeyMutation, 2> {
                auto& src = cur[0];
                auto& dst = cur[1];

                if (!src) return {Storage::Skip{}, Storage::Skip{}};

                auto* s = std::get_if<Storage::SetType>(&src->GetValue());
                if (!s) throw WrongTypeError{};
                if (s->count(member) == 0) return {Storage::Skip{}, Storage::Skip{}};

                const Storage::SetType* d = nullptr;
                if (dst) {
                    d = std::get_if<Storage::SetType>(&dst->GetValue());
                    if (!d) throw WrongTypeError{};
                }

                Storage::SetType new_src = *s;
                new_src.erase(member);

                Storage::SetType new_dst = d ? *d : Storage::SetType{};
                new_dst.insert(member);

                std::array<Storage::KeyMutation, 2> out;
                if (new_src.empty()) out[0] = Storage::Delete{};
                else out[0] = src->WithValue(std::move(new_src));

                if (dst) out[1] = dst->WithValue(new_dst);
                else out[1] = Storage::Entry{std::move(new_dst)};

                moved = true;
                return out;
            },
            src_key, dst_key);

        if (moved) ctx.Out().Int(1);
        else ctx.Out().Int(0);
    }
};

} // namespace Commands