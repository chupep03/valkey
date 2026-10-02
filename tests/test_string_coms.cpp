#include "commands/CommandReg.hpp"
#include "commands/RegisterAll.hpp"
#include "storage/MemoryManager.hpp"
#include "storage/StorageEngine.hpp"
#include "fake_response.hpp"

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

}

TEST(StringCommands, SetGet) {
    Fixture f;
    f.Run("SET", {"k", "hello"});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Ok);

    f.out.Clear();
    f.Run("GET", {"k"});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Bulk);
    EXPECT_EQ(f.out.Last().text, "hello");
}

TEST(StringCommands, GetMissing) {
    Fixture f;
    f.Run("GET", {"nope"});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Nil);
}

TEST(StringCommands, Append) {
    Fixture f;
    f.Run("SET", {"k", "foo"});
    f.out.Clear();
    f.Run("APPEND", {"k", "bar"});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Int);
    EXPECT_EQ(f.out.Last().integer, 6);
}

TEST(StringCommands, ExpireAndTtl) {
    Fixture f;
    f.Run("SET", {"k", "v"});
    f.out.Clear();
    f.Run("EXPIRE", {"k", "60"});
    EXPECT_EQ(f.out.Last().integer, 1);

    f.out.Clear();
    f.Run("TTL", {"k"});
    EXPECT_GE(f.out.Last().integer, 0);
    EXPECT_LE(f.out.Last().integer, 60);
}

TEST(StringCommands, TtlNoTtl) {
    Fixture f;
    f.Run("SET", {"k", "v"});
    f.out.Clear();
    f.Run("TTL", {"k"});
    EXPECT_EQ(f.out.Last().integer, -1);
}

TEST(StringCommands, TtlMissingKey) {
    Fixture f;
    f.Run("TTL", {"nope"});
    EXPECT_EQ(f.out.Last().integer, -2);
}