#include "StorageEngine.hpp"

#include <iostream>

namespace Storage {

    Entry* StorageEngine::Get(const std::string& key) {
        auto it = storage.find(key);
        
        if (it == storage.end()) {
            return nullptr;
        }

        if (it->second.exp_time.has_value()) {
            auto now = std::chrono::system_clock::now();
            if (now >= it->second.exp_time.value()) {
                mem_manager.Decrease(it->second.memory_usage);
                storage.erase(it);
                return nullptr;
            }
        }

        return &(it->second);
    }

    void StorageEngine::Set(const std::string& key, Entry&& entry) {
        size_t new_size = entry.CalculateSelfSize() + key.size();
        
        size_t old_size = 0;
        auto it = storage.find(key);
        
        if (it != storage.end()) {
            old_size = it->second.memory_usage;
        }

        if (!mem_manager.CanAllocate(new_size, old_size)) {
            throw std::runtime_error("StorageEngine.Set: Can't allocate memory!");
        }

        if (new_size > old_size) {
            mem_manager.Increase(new_size - old_size);
        } else {
            mem_manager.Decrease(old_size - new_size);
        }

        entry.memory_usage = new_size;
        storage[key] = std::move(entry);
    }

}

