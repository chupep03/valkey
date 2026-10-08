#pragma once

#include "commands/Context.hpp"
#include "storage/StorageEngine.hpp"

#include <string>

namespace Commands {

// SMOVE src dst member
// Moves member from source set to destination set
// If member is not in source -> int(0), nothing changes
// Otherwise -> int(1). Wrong type on either key -> WRONGTYPE
class SMoveCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireArgs(3);
        std::string src_key(ctx.GetArgumentAsStr(0));
        std::string dst_key(ctx.GetArgumentAsStr(1));
        std::string member(ctx.GetArgumentAsStr(2));

        switch (ctx.Storage().SMove(src_key, dst_key, member)) {
        case Storage::SMoveStatus::Moved:
            ctx.Out().Int(1);
            return;
        case Storage::SMoveStatus::NotAMember:
            ctx.Out().Int(0);
            return;
        case Storage::SMoveStatus::WrongType:
            throw WrongTypeError{};
        }
    }
};

} // namespace Commands