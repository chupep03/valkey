#pragma once 

#include "CommandReg.hpp"
#include "CommandsImport.hpp"


namespace Commands {

inline void RegisterAll(CommandReg& reg) {
    // String
    reg.RegisterCommand("SET",    std::make_unique<SetCommand>());
    reg.RegisterCommand("GET",    std::make_unique<GetCommand>());
    reg.RegisterCommand("STRLEN", std::make_unique<StrlenCommand>());
    reg.RegisterCommand("APPEND", std::make_unique<AppendCommand>());
    reg.RegisterCommand("EXPIRE", std::make_unique<ExpireCommand>());
    reg.RegisterCommand("TTL",    std::make_unique<TTLCommand>());

    // List
    reg.RegisterCommand("LPUSH",   std::make_unique<LPushCommand>());
    reg.RegisterCommand("RPUSH",   std::make_unique<RPushCommand>());
    reg.RegisterCommand("LPOP",    std::make_unique<LPopCommand>());
    reg.RegisterCommand("RPOP",    std::make_unique<RPopCommand>());
    reg.RegisterCommand("LLEN",    std::make_unique<LLenCommand>());
    reg.RegisterCommand("LRANGE",  std::make_unique<LRangeCommand>());
    reg.RegisterCommand("LINDEX",  std::make_unique<LIndexCommand>());
    reg.RegisterCommand("LSET",    std::make_unique<LSetCommand>());
    reg.RegisterCommand("LINSERT", std::make_unique<LInsertCommand>());

    // Set
    reg.RegisterCommand("SADD",      std::make_unique<SAddCommand>());
    reg.RegisterCommand("SREM",      std::make_unique<SRemoveCommand>());
    reg.RegisterCommand("SISMEMBER", std::make_unique<SIsMemberCommand>());
    reg.RegisterCommand("SMEMBERS",  std::make_unique<SMembersCommand>());
    reg.RegisterCommand("SCARD",     std::make_unique<SCardCommand>());
    reg.RegisterCommand("SUNION",    std::make_unique<SUnionCommand>());
    reg.RegisterCommand("SINTER",    std::make_unique<SInterCommand>());
    reg.RegisterCommand("SDIFF",     std::make_unique<SDiffCommand>());
    reg.RegisterCommand("SMOVE",     std::make_unique<SMoveCommand>());

    // Generics
    reg.RegisterCommand("TYPE",         std::make_unique<TypeCommand>());
    reg.RegisterCommand("DEL",          std::make_unique<DelCommand>());
    reg.RegisterCommand("EXISTS",       std::make_unique<ExistsCommand>());
    reg.RegisterCommand("KEYS",         std::make_unique<KeysCommand>());
    reg.RegisterCommand("FLUSHDB",      std::make_unique<FlushDbCommand>());
    reg.RegisterCommand("DBSIZE",       std::make_unique<DbSizeCommand>());
    reg.RegisterCommand("MEMORY",       std::make_unique<MemoryUsageCommand>());
    reg.RegisterCommand("CONFIG",       std::make_unique<ConfigCommand>());

    // Geo
    reg.RegisterCommand("GEOADD",         std::make_unique<GeoAddCommand>());
    reg.RegisterCommand("GEOPOS",         std::make_unique<GeoPosCommand>());
    reg.RegisterCommand("GEODIST",        std::make_unique<GeoDistCommand>());
    reg.RegisterCommand("GEOSEARCH",      std::make_unique<GeoSearchCommand>());
    reg.RegisterCommand("GEOSEARCHSTORE", std::make_unique<GeoSearchStoreCommand>());
}


} // namsepace Commands

