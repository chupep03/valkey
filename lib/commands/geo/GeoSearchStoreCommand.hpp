#pragma once

#include "commands/Context.hpp"
#include "commands/ICommand.hpp"

#include "GeoHelpers.hpp"
#include "GeoSearchHelpers.hpp"

#include <array>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// GEOSEARCHSTORE dst src FROMLONLAT lon lat BYRADIUS radius unit [ASC|DESC] [COUNT n]
// Stores the filtered by radius members names into set - dst
// Returns the number of stored members. Wrong type -> WRONGTYPE
class GeoSearchStoreCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(8);

        std::string dst_key(ctx.GetArgumentAsStr(0));
        std::string src_key(ctx.GetArgumentAsStr(1));
        auto q = GeoHelpers::ParseSearchTail(ctx, 2);

        if (dst_key == src_key) throw CommandException("GeoSearchStore: source and destination must differ");

        long long filtered_count = 0;

        ctx.Storage().DoMulti([&](auto currents) -> std::array<Storage::KeyMutation, 2> {
                auto src_entry = currents[0];
                auto dst_entry = currents[1];

                std::array<Storage::KeyMutation, 2> result{};
                result[0] = Storage::Skip{};

                if (dst_entry && !std::get_if<Storage::SetType>(&dst_entry->value)) {
                    throw WrongTypeError();
                }

                if (!src_entry) {
                    result[1] = Storage::Delete{};
                    filtered_count = 0;
                    return result;
                }

                auto* points = std::get_if<Storage::GeoType>(&src_entry->value);
                if (!points) throw WrongTypeError();
                auto filtered = GeoHelpers::RunSearch(*points, q);
                filtered_count = static_cast<long long>(filtered.size());

                if (filtered.empty()) {
                    result[1] = Storage::Delete{};
                    return result;
                }

                Storage::SetType members;
                members.reserve(filtered.size());
                for (const auto* p : filtered) {
                    members.insert(p->member);
                }

                if (dst_entry) result[1] = dst_entry->WithValue(std::move(members));
                else  result[1] = Storage::Entry{std::move(members)};
                return result;
            },
            src_key, dst_key);

        ctx.Out().Int(filtered_count);
    }
};

} // namespace Commands