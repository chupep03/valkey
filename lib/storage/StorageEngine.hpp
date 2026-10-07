#pragma once

#include "Entry.hpp"
#include "MemoryManager.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>

namespace Storage {

class StorageEngine {
private:
    using StorageMap =
        std::unordered_map<std::string, std::shared_ptr<const Entry>>;

    using Iterator = StorageMap::iterator;
    using ConstIterator = StorageMap::const_iterator;

    StorageMap storage_;
    MemoryManager& mem_manager_;
    mutable std::shared_mutex mtx_;

    // private helpers haven't lockers. Locking in public methods to avoid deadlock
    
    // Doesn't delete expired key
    ConstIterator FindLiveShared(const std::string& key) const {
        auto it = storage_.find(key);
        if (it == storage_.end()) return it;
        if (it->second->IsExpired()) return storage_.end();
        return it;
    }

    // Deletes expired key
    Iterator FindLiveUnique(const std::string& key) {
        auto it = storage_.find(key);
        if (it == storage_.end()) return it;
        if (!it->second->IsExpired()) return it;

        mem_manager_.Resize(TotalSizeFor(key, *it->second), 0);
        return storage_.end();
    }

    void PurgeExpired() {
        for (auto it = storage_.begin(); it != storage_.end();) {
            if (it->second->IsExpired()) {
                mem_manager_.Resize(TotalSizeFor(it->first, *it->second), 0);
                it = storage_.erase(it);
            } else {
                it++;
            }
        }
    }

    static size_t TotalSizeFor(const std::string& key, const Entry& e) {
        return key.size() + e.GetMemoryUsage();
    }

    static bool MatchPattern(const std::string& str, const std::string& pattern) {
        size_t s = 0, p = 0;
        size_t star_p = std::string::npos, star_s = 0;

        while (s < str.size()) {
            // solo simbol match
            if (p < pattern.size() &&
                (pattern[p] == '?' || pattern[p] == str[s])) {
                ++s; ++p;
            // * start
            } else if (p < pattern.size() && pattern[p] == '*') {
                star_p = p++;
                star_s = s;
            // * was in the past
            } else if (star_p != std::string::npos) {
                p = star_p + 1;
                s = ++star_s;
            // symbols dont match each other
            } else {
                return false;
            }
        }
        while (p < pattern.size() && pattern[p] == '*') ++p;
        return p == pattern.size();
    }

public:
    explicit StorageEngine(MemoryManager& mm)
        : mem_manager_(mm) {}

    std::shared_ptr<const Entry> Get(const std::string& key) {
        std::shared_lock lk(mtx_);
        auto it = FindLiveShared(key);
        if (it == storage_.end()) return nullptr;
        return it->second;
    }

    bool Exist(const std::string& key) {
        std::shared_lock lk(mtx_);
        return FindLiveShared(key) != storage_.end();
    }

    std::vector<std::string> GetAllKeysByPattern(const std::string& pattern) {
        std::unique_lock lk(mtx_);
        PurgeExpired();
        std::vector<std::string> result;
        for (const auto& kv : storage_) {
            if (MatchPattern(kv.first, pattern)) {
                result.push_back(kv.first);
            }
        }
        return result;
    }

    size_t GetMemUsage(const std::string& key) {
        std::shared_lock lk(mtx_);
        auto it = FindLiveShared(key);
        if (it == storage_.end()) return 0;
        return TotalSizeFor(key, *it->second);
    }

    size_t Size() {
        std::unique_lock lk(mtx_);
        PurgeExpired();
        return storage_.size();

    }

    void SetMaxMemory(size_t bytes) {
        std::unique_lock lk(mtx_);
        mem_manager_.SetLimit(bytes);
    }

    size_t TotalMemory() const {
        std::shared_lock lk(mtx_);
        return mem_manager_.GetUsage();
    }

    void Set(const std::string& key, Entry entry) {
        std::unique_lock lk(mtx_);
        auto it = FindLiveUnique(key);
        size_t old_total = 0; // if expired
        if (it != storage_.end()) old_total = TotalSizeFor(key, *it->second); 
        size_t new_total = TotalSizeFor(key, entry);

        mem_manager_.Resize(old_total, new_total);

        auto ptr = std::make_shared<const Entry>(std::move(entry));
        if (it != storage_.end()) it->second = std::move(ptr);
        else storage_.emplace(key, std::move(ptr));
    }

    int Remove(const std::vector<std::string>& keys) {
        std::unique_lock lk(mtx_);
        int removed = 0;
        for (const auto& key : keys) {
            auto it = FindLiveUnique(key);
            if (it == storage_.end()) continue; 
            mem_manager_.Resize(TotalSizeFor(key, *it->second), 0);
            storage_.erase(it);
            ++removed;
        }
        return removed;
    }

    void Flush() {
        // Invariant - mem_manager_.Usage == GetTotalSize for every element in storage
        std::unique_lock lk(mtx_);
        mem_manager_.Resize(mem_manager_.GetUsage(), 0);
        storage_.clear();
    }

    template<typename F>
    bool Do(const std::string& key, F&& f) {
        std::unique_lock lk(mtx_);

        auto it = FindLiveUnique(key);
        bool existed = (it != storage_.end());
        std::shared_ptr<const Entry> current;
        if (existed) current = it->second;
        std::optional<Entry> updated = f(std::move(current));

        size_t old_total = existed ? TotalSizeFor(key, *it->second) : 0;
        size_t new_total = updated ? TotalSizeFor(key, *updated) : 0;

        std::shared_ptr<const Entry> ptr;
        if (updated) ptr = std::make_shared<const Entry>(std::move(*updated));

        mem_manager_.Resize(old_total, new_total);

        if (ptr) {
            storage_.insert_or_assign(key, std::move(ptr));
        } else if (existed) {
            storage_.erase(it);
        }

        return existed;
    }
};

} // namespace Storage