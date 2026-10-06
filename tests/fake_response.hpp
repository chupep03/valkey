#pragma once

#include "commands/IResponse.hpp"

#include <optional>
#include <string>
#include <vector>

namespace Tests {


// Fake IRespose for tests
class FakeResponse : public Commands::IResponse {
public:
    enum class Kind { None, Ok, Nil, Int, Bulk, Array, Error };

    // One response
    struct Call {
        Kind kind = Kind::None;
        long long integer = 0;
        std::string text;
        std::vector<std::string> items;
    };

    std::vector<Call> calls;

    void Ok() override { calls.push_back({Kind::Ok, 0, "", {}}); }
    void Nil() override { calls.push_back({Kind::Nil, 0, "", {}}); }
    void Int(long long n) override { calls.push_back({Kind::Int, n, "", {}}); }
    void Bulk(const std::string& s) override { calls.push_back({Kind::Bulk, 0, s, {}}); }
    void Error(const std::string& msg) override { calls.push_back({Kind::Error, 0, msg, {}}); }
    void Array(const std::vector<std::string>& items) override {
        Call c; c.kind = Kind::Array; c.items = items;
        calls.push_back(std::move(c));
    }

    const Call& Last() const { return calls.back(); }
    bool Empty() const { return calls.empty(); }
    void Clear() { calls.clear(); }
};

} // namespace Tests