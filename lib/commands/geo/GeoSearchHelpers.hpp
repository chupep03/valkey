#pragma once

#include "GeoHelpers.hpp"
#include "../Context.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Commands::GeoHelpers {

struct SearchArguments {
    double lon = 0.0;
    double lat = 0.0;
    double radius_km = 0.0;
    bool asc = false; // from nearest to furthers;
    bool desc = false; // from further to nearest
    bool has_sort = false;
    bool has_count = false;
    long long count = 0;
};

// TAIL(start from LonLat etc) for GeoSearch and GeoSearchStore
inline SearchArguments ParseSearchTail(Context& ctx, size_t tail_start) {
    SearchArguments search_args;
    auto need = [&](size_t tag_idx, std::string tag, bool tag_check = false) {
        if (ctx.GetArgsCount() <= tag_idx) 
            throw CommandException(SyntaxError{}, "syntax error: expected " + tag);
        if (tag_check && (ToUpper(std::string(ctx.GetArgumentAsStr(tag_idx))) != tag)) 
            throw CommandException(SyntaxError{}, "syntax error: expected tagword " + tag);
    };

    // FROMLONLAT
    need(tail_start, "FROMLONLAT", true);
    need(tail_start + 1, "longitude");
    search_args.lon = ctx.GetArgumentAsDouble(tail_start + 1);
    need(tail_start + 2, "latitude");
    search_args.lat = ctx.GetArgumentAsDouble(tail_start + 2);

    // RADIUS
    need(tail_start + 3, "BYRADIUS", true);
    need(tail_start + 4, "radius");
    double radius = ctx.GetArgumentAsDouble(tail_start + 4);
    need(tail_start + 5, "unit");
    std::string unit = ToUpper(std::string(ctx.GetArgumentAsStr(tail_start + 5)));
    double unit_m = UnitToMeters(unit);

       if (radius < 0.0) throw CommandException(OtherError{}, "radius must be non-negative");
    search_args.radius_km = (radius * unit_m) / 1000.0;

    ValidateCoordinates(search_args.lon, search_args.lat);

    std::size_t i = tail_start + 6;

    // [Opt] ASC|DESC
    if (i < ctx.GetArgsCount()) {
        std::string opt = ToUpper(std::string(ctx.GetArgumentAsStr(i)));
        if (opt == "ASC")  { 
            search_args.has_sort = true;
            search_args.asc = true;
            i++;
        }
        else if (opt == "DESC") {
            search_args.has_sort = true;
            search_args.desc = true;
            i++;
        }
    }

    // [Opt] COUNT N
    if (i < ctx.GetArgsCount()) {
        std::string opt = ToUpper(std::string(ctx.GetArgumentAsStr(i)));
        if (opt != "COUNT")
            throw CommandException(SyntaxError{}, "syntax error: unexpected token '" +
                               std::string(ctx.GetArgumentAsStr(i)) + "'");
        if (i + 1 >= ctx.GetArgsCount()) 
            throw CommandException(SyntaxError{}, "syntax error: COUNT requires a value");

        long long n = ctx.GetArgumentAsLLInt(i + 1);
        if (n <= 0) throw CommandException(SyntaxError{}, "COUNT must be positive");
        search_args.has_count = true;
        search_args.count = n;
        i += 2;
    }

    if (i != ctx.GetArgsCount()) throw CommandException(SyntaxError{}, "syntax error: trailing tokens");
    return search_args;
}

// FilterByRadius
inline std::vector<std::string> RunSearch(const Storage::GeoType& points, const SearchArguments& args) {
    std::vector<std::pair<double, std::string>> hits;
    hits.reserve(points.size());

    for (auto& p : points) {
        double d = HaversineKm(args.lat, args.lon, p.latitude, p.longitude);
        if (d <= args.radius_km) hits.emplace_back(d, p.member);
    }

    if (args.has_sort) {
        std::sort(hits.begin(), hits.end(),
            [asc = args.asc](const auto& a, const auto& b) {
                return asc ? a.first < b.first : a.first > b.first;
            });
    }

    if (args.has_count && static_cast<long long>(hits.size()) > args.count) {
        hits.resize(static_cast<std::size_t>(args.count));
    }

    std::vector<std::string> out;
    out.reserve(hits.size());
    for (auto& [_, m] : hits) out.push_back(std::move(m));
    return out;
}

    
} // namespace Commands::GeoHelpers
