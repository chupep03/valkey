#pragma once

#include "ValueVariant.hpp"
#include "Overloaded.hpp"

#include <optional>
#include <chrono>
#include <cstddef>

namespace Storage {
using t_point = std::chrono::system_clock::time_point;

class Entry {
private:
    size_t memory_usage_ = 0;
    ValueVariant value_; // immutable
    std::optional<t_point> exp_time_;

public:    
    Entry() = default;

    const ValueVariant& GetValue() const { return value_; }
    const std::optional<t_point>& GetExpTime() const { return exp_time_; }

    explicit Entry(ValueVariant val, std::optional<t_point> expire = std::nullopt)
        : value_(std::move(val)), exp_time_(expire) {
        memory_usage_ = CalculateSelfSize();
    }

    size_t GetMemoryUsage() const {return memory_usage_;}

    bool IsExpired() const {
        if (!exp_time_.has_value()) return false;
        return std::chrono::system_clock::now() >= exp_time_.value();
    }

    size_t CalculateSelfSize() const {
        size_t size = sizeof(Entry);
        auto StrAdd = [&](const StringType& s) {size += sizeof(std::string) + s.capacity();};
        auto SetAdd = [&](const SetType& set) {for (const auto& s : set) size += kHashNodeOverhead + sizeof(std::string) + s.capacity();};
        auto ListAdd = [&](const ListType& list) {for (const auto& l : list) size += sizeof(std::string) + l.capacity();};
        auto GeoAdd = [&](const GeoType& geo) {
            size += geo.capacity() * sizeof(GeoPoint);
            for (const auto& g : geo) size += g.member.capacity();
        };

        std::visit(Overloaded{StrAdd, SetAdd, ListAdd, GeoAdd}, value_);

        return size;
    }

    Entry WithValue(ValueVariant new_value) const {
        return Entry{std::move(new_value), exp_time_};
    }
};
}