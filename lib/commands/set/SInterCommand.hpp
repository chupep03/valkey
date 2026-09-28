#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Commands {

// SINTER key [key ...]
// Returns the intersection of the given sets. Missing keys count as empty
// Order is unspecified. Wrong type on any key -> WRONGTYPE
class SInterCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        // todo
    }
};

} // namespace Commands