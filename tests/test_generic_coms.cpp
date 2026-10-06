#include "commands/CommandReg.hpp"
#include "commands/RegisterAll.hpp"
#include "storage/MemoryManager.hpp"
#include "storage/StorageEngine.hpp"
#include "fake_response.hpp"

#include <algorithm>
#include <gtest/gtest.h>

using namespace Commands;
using Storage::MemoryManager;
using Storage::StorageEngine;

namespace {

struct Fixture {
    MemoryManager mm;
    StorageEngine engine{mm};
    CommandReg reg;
    Tests::FakeResponse out;

    Fixture() { RegisterAll(reg); }

    void Run(const std::string& name, std::vector<std::string> args) {
        auto* cmd = reg.FindCommand(name);
        ASSERT_NE(cmd, nullptr) << "no such command: " << name;
        Context ctx(name, std::move(args), engine, mm, out);
        cmd->Execute(ctx);
    }
};

} // namespace

TEST(GenericCommands, TypeAllKinds) {
    Fixture f;
    f.Run("SET",  {"s", "v"});
    f.Run("RPUSH",{"l", "x"});
    f.Run("SADD", {"st", "m"});
    f.Run("GEOADD", {"g", "30", "60", "p"});

    f.out.Clear(); f.Run("TYPE", {"s"});
    EXPECT_EQ(f.out.Last().text, "string");
    f.out.Clear(); f.Run("TYPE", {"l"});
    EXPECT_EQ(f.out.Last().text, "list");
    f.out.Clear(); f.Run("TYPE", {"st"});
    EXPECT_EQ(f.out.Last().text, "set");
    f.out.Clear(); f.Run("TYPE", {"g"});
    EXPECT_EQ(f.out.Last().text, "zset");
    f.out.Clear(); f.Run("TYPE", {"missing"});
    EXPECT_EQ(f.out.Last().text, "none");
}

TEST(GenericCommands, DelAndExistsCountDifferently) {
    Fixture f;
    f.Run("SET", {"a", "1"});
    f.Run("SET", {"b", "2"});

    f.out.Clear(); f.Run("EXISTS", {"a", "a", "b"});
    EXPECT_EQ(f.out.Last().integer, 3);

    f.out.Clear(); f.Run("DEL", {"a", "a", "b"});
    EXPECT_EQ(f.out.Last().integer, 2);
}

TEST(GenericCommands, KeysGlob) {
    Fixture f;
    f.Run("SET", {"user:1", "a"});
    f.Run("SET", {"user:2", "b"});
    f.Run("SET", {"post:1", "c"});

    f.out.Clear(); f.Run("KEYS", {"user:*"});
    auto v = f.out.Last().items;
    std::sort(v.begin(), v.end());
    EXPECT_EQ(v, (std::vector<std::string>{"user:1", "user:2"}));

    f.out.Clear(); f.Run("KEYS", {"user:?"});
    EXPECT_EQ(f.out.Last().items.size(), 2u);
}

TEST(GenericCommands, DbSizeAndFlush) {
    Fixture f;
    f.Run("SET", {"a", "1"});
    f.Run("SET", {"b", "2"});
    f.Run("SET", {"c", "3"});

    f.out.Clear(); f.Run("DBSIZE", {});
    EXPECT_EQ(f.out.Last().integer, 3);

    f.out.Clear(); f.Run("FLUSHDB", {});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Ok);

    f.out.Clear(); f.Run("DBSIZE", {});
    EXPECT_EQ(f.out.Last().integer, 0);
}

TEST(GenericCommands, ConfigMaxmemory) {
    Fixture f;
    f.out.Clear(); f.Run("CONFIG", {"GET", "maxmemory"});
    ASSERT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Array);
    ASSERT_EQ(f.out.Last().items.size(), 2u);
    EXPECT_EQ(f.out.Last().items[0], "maxmemory");
    EXPECT_EQ(f.out.Last().items[1], "0");

    f.out.Clear(); f.Run("CONFIG", {"SET", "maxmemory", "1mb"});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Ok);

    f.out.Clear(); f.Run("CONFIG", {"GET", "maxmemory"});
    EXPECT_EQ(f.out.Last().items[1], std::to_string(1024u * 1024u));
}