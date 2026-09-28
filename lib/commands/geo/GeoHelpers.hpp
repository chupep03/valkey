#pragma once

#include "../Context.hpp"

#include <cmath>
#include <string>

namespace Commands::GeoHelpers {

inline constexpr double kEarthRadiusKm = 6372.8; 
inline constexpr double kPi = 3.14159265358979323846;
inline constexpr double kMaxLat = 85.05112878;

// Great-circle distance between two points, in kilometers
// Haversine formula
inline double HaversineKm(double lat1, double lon1,
                          double lat2, double lon2) noexcept {
    auto toRad = [](double d) { return d * kPi / 180.0; };
    const double dLat = toRad(lat2 - lat1);
    const double dLon = toRad(lon2 - lon1);
    const double a =
        std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(toRad(lat1)) * std::cos(toRad(lat2)) *
        std::sin(dLon / 2) * std::sin(dLon / 2);
    return kEarthRadiusKm * 2.0 * std::asin(std::sqrt(a));
}

inline double UnitToMeters(const std::string& unit) {
    if (unit == "m")  return 1.0;
    if (unit == "km") return 1000.0;
    if (unit == "mi") return 1609.344;
    if (unit == "ft") return 0.3048;
    throw CommandException(OtherError{}, "unsupported unit '" + unit + "'");
}

inline std::string ToUpper(std::string s) {
    for (char& c : s) c = static_cast<char>(
        std::toupper(static_cast<unsigned char>(c)));
    return s;
}

inline void ValidateCoordinates(double lon, double lat) {
    if (lon < -180.0 || lon > 180.0)
        throw CommandException(OtherError{}, "invalid longitude, must be in [-180, 180]");
    if (lat < -kMaxLat || lat > kMaxLat)
        throw CommandException(OtherError{}, "invalid latitude, must be in [-85.05112878, 85.05112878]");
}

} // namespace Commands::GeoHelpers