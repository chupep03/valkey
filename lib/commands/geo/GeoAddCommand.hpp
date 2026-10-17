#pragma once

#include "../Context.hpp"
#include "GeoHelpers.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace Commands {

// GEOADD key lon lat member [lon lat member ...]
// Adds or updates geo points. Existing members are updated, not duplicated
// Coordinates must be valid: lon in [-180,180], lat in [-85.05,85.05]
// Replies with int(number of new points added). Wrong type -> WRONGTYPE
class GeoAddCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(4);

        const std::size_t rest = ctx.GetArgsCount() - 1;
        if (rest % 3 != 0) throw CommandException("syntax error");

        std::string key(ctx.GetArgumentAsStr(0));
        long long added = 0;

        ctx.Storage().Do(key, [&](std::shared_ptr<const Storage::Entry> current) -> std::optional<Storage::Entry> {
                Storage::GeoType points;
                if (current) {
                    auto* p = std::get_if<Storage::GeoType>(&current->GetValue());
                    if (!p) throw WrongTypeError{};
                    points = *p;
                }

                for (std::size_t i = 1; i + 3 <= ctx.GetArgsCount(); i += 3) {
                    double lon = ctx.GetArgumentAsDouble(i);
                    double lat = ctx.GetArgumentAsDouble(i + 1);
                    std::string member(ctx.GetArgumentAsStr(i + 2));

                    GeoHelpers::ValidateCoordinates(lon, lat);

                    bool found = false;
                    for (auto& p : points) {
                        if (p.member == member) {
                            p.longitude = lon;
                            p.latitude  = lat;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        points.push_back({lon, lat, std::move(member)});
                        ++added;
                    }
                }

                if (current) return current->WithValue(std::move(points));
                return Storage::Entry{std::move(points)};
            });

        ctx.Out().Int(added);
    }
};

} // namespace Commands