#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <cctype>

#include "StorageException.hpp"

namespace Storage {

inline static constexpr size_t KB = 1024ULL;
inline static constexpr size_t MB = 1024ULL * KB;
inline static constexpr size_t GB = 1024ULL * MB;

class MemoryManager {
private:
    size_t actual_usage_ = 0;
    size_t usage_limit_  = 0;   // 0 - no limit

public:
    MemoryManager(size_t limit_bytes = 0) noexcept : usage_limit_(limit_bytes) {}

    size_t GetUsage() const noexcept {return actual_usage_;}
    size_t GetLimit() const noexcept {return usage_limit_;}

     void SetLimit(size_t bytes) {
        if (bytes != 0 && bytes < actual_usage_)
            throw std::invalid_argument("MemoryManager::SetLimit: limit < usage");
        usage_limit_ = bytes;
    }
    bool CanAllocate(size_t new_size, size_t old_size = 0) const noexcept {
        if (old_size > actual_usage_) return false;
        if (usage_limit_ == 0) return true;
        size_t base = actual_usage_ - old_size;
        return new_size <= usage_limit_ - base;
    }

    void Resize(size_t old_size, size_t new_size) {
        if (!CanAllocate(new_size, old_size)) throw OutOfMemoryException();
        if (new_size > old_size) actual_usage_ += (new_size - old_size);
        else if (new_size < old_size) actual_usage_ -= (old_size - new_size);
    }

    static size_t ParseSizeToBytes(const std::string& str) {
        if (str.empty()) throw ParseException("nothing to parse");

        size_t index = 0;
        while (index < str.length() && std::isdigit(static_cast<u_char>(str[index]))) index++;
        std::string number_str = str.substr(0, index);

        std::string postfix = str.substr(index);
        for (char& c : postfix) 
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        if (number_str.empty()) throw ParseException("number expected");
        size_t number = static_cast<size_t>(std::stoull(number_str));

        size_t multiplier = 1;
        if (postfix.empty() || postfix == "B")      multiplier = 1;
        else if (postfix == "KB" || postfix == "K") multiplier = KB;
        else if (postfix == "MB" || postfix == "M") multiplier = MB;
        else if (postfix == "GB" || postfix == "G") multiplier = GB;
        else throw ParseException("unknown postfix '" + postfix + "'");

        return number * multiplier;
    }

};

} // namespace Storage