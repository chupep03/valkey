#pragma once 

#include "CommandReg.hpp"

#include "string/SetCommand.hpp"
#include "string/GetCommand.hpp"
#include "string/StrlenCommand.hpp"
#include "string/AppendCommand.hpp"
#include "string/ExpireCommand.hpp"
#include "string/TTLCommand.hpp"

namespace Commands {

inline void RegisterAll(CommandReg& reg) {
    reg.RegisterCommand("SET",    std::make_unique<SetCommand>());
    reg.RegisterCommand("GET",    std::make_unique<GetCommand>());
    reg.RegisterCommand("STRLEN", std::make_unique<StrlenCommand>());
    reg.RegisterCommand("APPEND", std::make_unique<AppendCommand>());
    reg.RegisterCommand("EXPIRE", std::make_unique<ExpireCommand>());
    reg.RegisterCommand("TTL",    std::make_unique<TTLCommand>());
}

} // namsepace Commands

