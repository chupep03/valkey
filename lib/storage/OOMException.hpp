#pragma once

#include <stdexcept>

namespace Storage {

class OOMException : public std::runtime_error {
public:
    OOMException() : std::runtime_error("OOM") {}
};

} // namespace Storage
