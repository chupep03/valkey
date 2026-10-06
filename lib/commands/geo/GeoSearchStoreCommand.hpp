#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include "GeoHelpers.hpp"
#include "GeoSearchHelpers.hpp"

#include <string>
#include <variant>
#include <vector>

namespace Commands {

// GEOSEARCHSTORE dest source FROMLONLAT lon lat BYRADIUS rad unit [ASC|DESC] [COUNT n]
// Same as GEOSEARCH but stores the result in dest
// Replies with Int(number of members stored). Wrong type -> WRONGTYPE
class GeoSearchStoreCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(9);
        std::string dest_key(ctx.GetArgumentAsStr(0));
        std::string src_key(ctx.GetArgumentAsStr(1));
        auto q = GeoHelpers::ParseSearchTail(ctx, 2);

        auto src = ctx.Storage().Get(src_key);
        if (!src) {
            Storage::SetType empty;
            ctx.Storage().Set(dest_key, Storage::Entry{std::move(empty)});
            ctx.Out().Int(0);
            return;
        }

        auto* points = std::get_if<Storage::GeoType>(&src->value);
        if (!points) throw WrongTypeError();
        std::vector<std::string> found = GeoHelpers::RunSearch(*points, q);
        Storage::SetType result(found.begin(), found.end());

        ctx.Storage().Set(dest_key, Storage::Entry{std::move(result)});
        ctx.Out().Int(static_cast<long long>(found.size()));
    }
};

} // namespace Commands