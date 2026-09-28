#pragma once

#include "../Context.hpp"
#include "../ICommand.hpp"
#include "../../storage/MemoryManager.hpp"

#include <cctype>
#include <string>
#include <vector>

namespace Commands {

// CONFIG SET maxmemory <bytes>
// CONFIG GET maxmemory
// Two-word command: args[0] is SET or GET, args[1] is the parameter
// SET: replies OK or an error if the new limit is below current usage
// GET: replies with Array ["maxmemory", "<value>"]
class ConfigCommand : public ICommand {
public:
    void Execute(Context& ctx) override {
        ctx.RequireMinArgs(2);
        std::string subcommand = ToUpper(std::string(ctx.GetArgumentAsStr(0)));
        std::string param  = ToLower(std::string(ctx.GetArgumentAsStr(1)));

        if (subcommand == "SET") {
            if (ctx.GetArgsCount() != 3) {
                throw CommandException(OtherError{}, "wrong number of arguments for 'config|set' command");
            }
            if (param != "maxmemory") {
                throw CommandException(OtherError{}, "Unknown option or number of arguments for CONFIG SET - '" + param + "'");
            }

            std::string value(ctx.GetArgumentAsStr(2));
            std::size_t bytes = 0;

            try {
                bytes = Storage::MemoryManager::ParseSizeToBytes(value);
            } catch (const std::exception& e) {
                throw CommandException(OtherError{}, std::string("bad maxmemory value: ") + e.what());
            }
            try {
                ctx.Memory().SetLimit(bytes);
            } catch (const std::invalid_argument& e) {
                throw CommandException(OtherError{}, e.what());
            }
            ctx.Out().Ok();
            return;
        }

        if (subcommand == "GET") {
            if (ctx.GetArgsCount() != 2) {
                throw CommandException(OtherError{}, "wrong number of arguments for CONFIG GET command");
            }
            if (param != "maxmemory") {
                throw CommandException(OtherError{}, "Unknown option or number of arguments for CONFIG GET - '" + param + "'");
            }
            ctx.Out().Array({"maxmemory", std::to_string(ctx.Memory().GetLimit())});
            return;
        }

        throw CommandException(OtherError{}, "unknown CONFIG subcommand or wrong number of arguments");
    }

private:
    static std::string ToUpper(std::string s) {
        for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    }
    static std::string ToLower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }
};

} // namespace Commands