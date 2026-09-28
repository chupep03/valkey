#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"
#include "GeoHelpers.hpp"
#include "GeoSearchHelpers.hpp"

#include <string>
#include <variant>
#include <vector>

namespace Commands {

// GEOSEARCH key FROMLONLAT lon lat BYRADIUS radius unit [ASC|DESC] [COUNT n]
// Returns members within the radius from the given point
// Replies with Array of member names. Wrong type -> WRONGTYPE
class GeoSearchCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(8);

        std::string key(ctx.GetArgumentAsStr(0));
        auto q = GeoHelpers::ParseSearchTail(ctx, 1);

        auto entry = ctx.Storage().Get(key);
        if (!entry) {
            ctx.Out().Array({});
            return;
        }
        auto* points = std::get_if<Storage::GeoType>(&entry->value);
        if (!points) throw CommandException(WrongTypeError{});

        std::vector<std::string> out = GeoHelpers::RunSearch(*points, q);
        ctx.Out().Array(out);
    }
};

} // namespace Commands