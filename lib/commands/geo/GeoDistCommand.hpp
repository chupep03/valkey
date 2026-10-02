#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"
#include "GeoHelpers.hpp"

#include <string>
#include <variant>

namespace Commands {

// GEODIST key member1 member2 [unit]
// Returns the distance between two members using the haversine formula
// Unit: m (default), km, mi, ft. Missing member -> nil
// Replies with Bulk. Wrong type -> WRONGTYPE
class GeoDistCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(3);
        std::string key(ctx.GetArgumentAsStr(0));
        std::string m1(ctx.GetArgumentAsStr(1));
        std::string m2(ctx.GetArgumentAsStr(2));
        std::string unit = "m";

        if (ctx.GetArgsCount() >= 4) {
            unit = GeoHelpers::ToUpper(std::string(ctx.GetArgumentAsStr(3)));
        }
        double unit_m = GeoHelpers::UnitToMeters(unit);

        auto entry = ctx.Storage().Get(key);
        if (!entry) { ctx.Out().Nil(); return;}
        auto* points = std::get_if<Storage::GeoType>(&entry->value);
        if (!points) throw WrongTypeError();

        const Storage::GeoPoint* p1 = nullptr;
        const Storage::GeoPoint* p2 = nullptr;
        for (const auto& p : *points) {
            if (p.member == m1) p1 = &p;
            if (p.member == m2) p2 = &p;
        }
        if (!p1 || !p2) { ctx.Out().Nil(); return; }

        double km = GeoHelpers::HaversineKm(p1->latitude, p1->longitude, p2->latitude, p2->longitude);
        double meters = km * 1000.0;
        double value = meters / unit_m;

        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.4f", value);
        ctx.Out().Bulk(buf);
    }
};

} // namespace Commands