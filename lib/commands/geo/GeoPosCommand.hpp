#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// GEOPOS key member [member ...]
// Returns the coordinates of each member as a "lon,lat" string
// Missing members produce a "(nil)" entry in the Array
// Replies with Array. Wrong type -> WRONGTYPE
class GeoPosCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(2);
        std::string key(ctx.GetArgumentAsStr(0));
        auto entry = ctx.Storage().Get(key);
        std::vector<std::string> result;
        result.reserve(ctx.GetArgsCount() - 1);

        if (!entry) {
            for (std::size_t i = 1; i < ctx.GetArgsCount(); i++) result.emplace_back("(nil)");
            ctx.Out().Array(result);
            return;
        }

        auto* points = std::get_if<Storage::GeoType>(&entry->value);
        if (!points) throw WrongTypeError();

        for (std::size_t i = 1; i < ctx.GetArgsCount(); i++) {
            std::string member(ctx.GetArgumentAsStr(i));
            const Storage::GeoPoint* found = nullptr;
            for (const auto& p : *points) {
                if (p.member == member) { found = &p; break; }
            }
            if (!found) {
                result.emplace_back("(nil)");
            } else {
                result.push_back(std::to_string(found->longitude) + "," +
                                 std::to_string(found->latitude));
            }
        }
        ctx.Out().Array(result);
    }
};

} // namespace Commands