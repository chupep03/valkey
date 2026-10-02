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

    std::vector<std::string> SortedLastArray() {
        auto v = out.Last().items;
        std::sort(v.begin(), v.end());
        return v;
    }
};

} // namespace

TEST(SetCommands, AddIgnoresDuplicates) {
    Fixture f;
    f.Run("SADD", {"s", "a", "b", "a", "c"});
    EXPECT_EQ(f.out.Last().integer, 3);

    f.out.Clear();
    f.Run("SCARD", {"s"});
    EXPECT_EQ(f.out.Last().integer, 3);

    f.out.Clear();
    f.Run("SISMEMBER", {"s", "b"});
    EXPECT_EQ(f.out.Last().integer, 1);
    f.out.Clear();
    f.Run("SISMEMBER", {"s", "z"});
    EXPECT_EQ(f.out.Last().integer, 0);
}

TEST(SetCommands, RemAndEmptyRemovesKey) {
    Fixture f;
    f.Run("SADD", {"s", "a", "b"});
    f.out.Clear();
    f.Run("SREM", {"s", "a", "missing"});
    EXPECT_EQ(f.out.Last().integer, 1);

    f.out.Clear();
    f.Run("SREM", {"s", "b"});
    EXPECT_EQ(f.out.Last().integer, 1);
    EXPECT_FALSE(f.engine.Exist("s"));
}

TEST(SetCommands, UnionIntersectionDiff) {
    Fixture f;
    f.Run("SADD", {"a", "x", "y", "z"});
    f.Run("SADD", {"b", "y", "z", "w"});

    f.out.Clear();
    f.Run("SUNION", {"a", "b"});
    EXPECT_EQ(f.SortedLastArray(),
              (std::vector<std::string>{"w", "x", "y", "z"}));

    f.out.Clear();
    f.Run("SINTER", {"a", "b"});
    EXPECT_EQ(f.SortedLastArray(),
              (std::vector<std::string>{"y", "z"}));

    f.out.Clear();
    f.Run("SDIFF", {"a", "b"});
    EXPECT_EQ(f.SortedLastArray(),
              (std::vector<std::string>{"x"}));
}

TEST(SetCommands, SMoveBetweenSets) {
    Fixture f;
    f.Run("SADD", {"src", "a", "b"});
    f.Run("SADD", {"dst", "c"});

    f.out.Clear();
    f.Run("SMOVE", {"src", "dst", "a"});
    EXPECT_EQ(f.out.Last().integer, 1);

    f.out.Clear();
    f.Run("SISMEMBER", {"src", "a"});
    EXPECT_EQ(f.out.Last().integer, 0);

    f.out.Clear();
    f.Run("SISMEMBER", {"dst", "a"});
    EXPECT_EQ(f.out.Last().integer, 1);

    // Move a member not in source -> 0.
    f.out.Clear();
    f.Run("SMOVE", {"src", "dst", "nope"});
    EXPECT_EQ(f.out.Last().integer, 0);
}

TEST(SetCommands, WrongType) {
    Fixture f;
    f.Run("SET", {"s", "x"});
    EXPECT_THROW(f.Run("SADD", {"s", "y"}), WrongTypeError);

    f.Run("SADD", {"real", "m"});
    EXPECT_THROW(f.Run("SADD", {"real", "m"}), WrongTypeError);
    f.out.Clear();
    f.Run("SADD", {"real", "m"});
    EXPECT_EQ(f.out.Last().integer, 0);
}