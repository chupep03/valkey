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

enum class SMoveStatus {
    Moved,       // member was in src, moved to dst - int(1)
    NotAMember,  // src missing, or member not in src - int(0)
    WrongType,
};

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

    // f - lambda `[](std::shared_ptr<const Entry>) -> std::optional<Entry>`
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


    SMoveStatus SMove(const std::string& src_key, const std::string& dst_key, const std::string& member) {
        std::unique_lock lk(mtx_);

        if (src_key == dst_key) {
            auto it = FindLiveUnique(src_key);
            if (it == storage_.end()) return SMoveStatus::NotAMember;
            auto* s = std::get_if<SetType>(&it->second->value);
            if (!s) return SMoveStatus::WrongType;
            return s->count(member) ? SMoveStatus::Moved : SMoveStatus::NotAMember;
        }

        auto src_it = FindLiveUnique(src_key);
        auto dst_it = FindLiveUnique(dst_key);

        // src checks
        if (src_it == storage_.end()) return SMoveStatus::NotAMember;
        auto* src = std::get_if<SetType>(&src_it->second->value);
        if (!src) return SMoveStatus::WrongType;
        if (src->count(member) == 0) return SMoveStatus::NotAMember;

        // dst checks
        const SetType* dst = nullptr;
        if (dst_it != storage_.end()) {
            dst = std::get_if<SetType>(&dst_it->second->value);
            if (!dst) return SMoveStatus::WrongType;
        }

        // new src
        SetType new_src = *src;
        new_src.erase(member);
        std::optional<Entry> new_src_entry;
        if (!new_src.empty()) {
            new_src_entry.emplace(src_it->second->WithValue(std::move(new_src)));
        }

        // new dst
        SetType new_dst = dst ? *dst : SetType{};
        new_dst.insert(member);
        Entry new_dst_entry{std::move(new_dst)};

        // old sizes valuation
        size_t old_src_total = TotalSizeFor(src_key, *src_it->second);
        size_t old_dst_total = 0;
        if (dst_it != storage_.end()) old_dst_total = TotalSizeFor(dst_key, *dst_it->second);

        // new sizes valuation
        size_t new_src_total = 0;
        if (new_src_entry) new_src_total = TotalSizeFor(src_key, *new_src_entry);
        size_t new_dst_total = TotalSizeFor(dst_key, new_dst_entry);

        mem_manager_.Resize(old_src_total + old_dst_total, new_src_total + new_dst_total);

        if (new_src_entry) {
            storage_.insert_or_assign(src_key,
                std::make_shared<const Entry>(std::move(*new_src_entry)));
        } else {
            storage_.erase(src_key);
        }
        storage_.insert_or_assign(dst_key,
            std::make_shared<const Entry>(std::move(new_dst_entry)));

        return SMoveStatus::Moved;
    }
};



} // namespace Storage