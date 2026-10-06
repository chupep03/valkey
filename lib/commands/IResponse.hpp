#pragma once

#include <string>
#include <vector>

namespace Commands {

class IResponse {
public:
    virtual ~IResponse() = default;

    virtual void Ok() = 0;
    virtual void Nil() = 0;
    virtual void Int(long long n) = 0;
    virtual void Bulk(const std::string& s) = 0;
    virtual void Array(const std::vector<std::string>& items) = 0;
    virtual void Error(const std::string& msg) = 0;
};

} // namespace Commands