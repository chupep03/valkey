#pragma once

#include <cstddef>
#include <exception>
#include <string>
#include <string_view>

#include "../storage/MemoryManager.hpp"

namespace Application {

struct Options {
    size_t maxmemory = 0;
    bool help_requested = false;
    bool parse_error = false;
    std::string error_message;
};

inline constexpr const char* kUsage =
    "Usage: valkey [--maxmemory <bytes>]\n"
    "  --maxmemory accepts suffixes: b, kb, mb, gb (e.g. 64mb).\n"
    "  0 means no limit (default).\n";

inline Options ParseArgs(int argc, char** argv) {
    Options opts;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            opts.help_requested = true;
        }
        else if (arg == "--maxmemory") {
            if (i + 1 >= argc) {
                opts.parse_error = true;
                opts.error_message = "--maxmemory requires a value";
                return opts;
            }
            std::string value = argv[++i];
            try {
                opts.maxmemory = Storage::MemoryManager::ParseSizeToBytes(value);
            }
            catch (const std::exception& e) {
                opts.parse_error = true;
                opts.error_message =
                    std::string("bad --maxmemory value: ") + e.what();
                return opts;
            }
        }
        else {
            opts.parse_error = true;
            opts.error_message = "unknown argument: " + std::string(arg);
            return opts;
        }
    }

    return opts;
}

} // namespace Application