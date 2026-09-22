#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include "Entry.hpp"
#include "MemoryManager.hpp"


namespace Storage {

    class StorageEngine {
    private:
        std::unordered_map<std::string, Entry> storage;
        MemoryManager& mem_manager;
        void check_ttl(const std::string& key);

    public:
        explicit StorageEngine(MemoryManager& mm) : mem_manager(mm) {}

        Entry* Get(const std::string& key);
        void Set(const std::string& key, Entry&& entry);
        size_t Size() const { return storage.size(); }

        int Remove(const std::vector<std::string>& keys);
        bool Exist(const std::string& key);
        std::vector<std::string> GetAllKeys(const std::string& pattern) const;

        void Flush();

        size_t GetMemUsage(const std::string& key);
        size_t TotalMemory() const;
    };
}
