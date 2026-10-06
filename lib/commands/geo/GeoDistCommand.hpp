#pragma once


#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

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
            unit = std::string(ctx.GetArgumentAsStr(3));
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

        double km = GeoHelpers::CalculateDistance(p1->latitude, p1->longitude, p2->latitude, p2->longitude);
        double meters = km * 1000.0;
        double value = meters / unit_m;
        std::string s = std::format("{:.4f}", value);
        ctx.Out().Bulk(s);
    }
};

} // namespace Commands