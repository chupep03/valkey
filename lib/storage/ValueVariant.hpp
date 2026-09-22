#include <string>
#include <vector>
#include <deque>
#include <unordered_set>
#include <variant>

namespace Storage {
    struct GeoPoint {double longitude, latitude; std::string member; };
    using StringType = std::string;
    using ListType = std::deque<std::string>;
    using SetType = std::unordered_set<std::string>;
    using GeoType = std::vector<GeoPoint>;
    using ValueVariant = std::variant<StringType, ListType, SetType, GeoType>;

    inline constexpr size_t kHashNodeOverhead = 32; // for set

} // namespace Storage