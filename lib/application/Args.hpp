#pragma once

#include <cstddef>
#include <exception>
#include <string>
#include <string_view>


#include "ApplicationException.hpp"
#include "storage/MemoryManager.hpp"

namespace Application {

struct Options {
    size_t maxmemory = 0;
    bool help_requested = false;
};

inline constexpr const char* kHelpResponse =
    "Usage: valkey [--maxmemory <bytes>]\n"
    "  --maxmemory suffixes: b, kb, mb, gb.\n"
    "  0 means no limit.\n";

inline Options ParseArgs(int argc, char** argv) {
    Options opts;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            opts.help_requested = true;
        }
        else if (arg == "--maxmemory") {
            if (i + 1 >= argc) {
                throw ParseException("--maxmemory reqires a value");
            }
            std::string value = argv[++i];
            try {
                opts.maxmemory = Storage::MemoryManager::ParseSizeToBytes(value);
            }
            catch (const std::exception& e) {
                throw ParseException("bad --maxmemory value: " + value);
            }
        }
        else {
            throw ParseException("unknown argument: " + std::string(arg));
        }
    }

    return opts;
}

} // namespace Application