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

} // namespace

TEST(ListCommands, PushPopBothEnds) {
    Fixture f;
    f.Run("RPUSH", {"k", "a", "b", "c"});
    EXPECT_EQ(f.out.Last().integer, 3);

    f.out.Clear();
    f.Run("LPUSH", {"k", "x"});
    EXPECT_EQ(f.out.Last().integer, 4);

    f.out.Clear();
    f.Run("LPOP", {"k"});
    EXPECT_EQ(f.out.Last().text, "x");

    f.out.Clear();
    f.Run("RPOP", {"k"});
    EXPECT_EQ(f.out.Last().text, "c");
}

TEST(ListCommands, LRangeWithNegativeIndexes) {
    Fixture f;
    f.Run("RPUSH", {"k", "a", "b", "c", "d"});
    f.out.Clear();
    f.Run("LRANGE", {"k", "1", "-1"});
    ASSERT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Array);
    EXPECT_EQ(f.out.Last().items, (std::vector<std::string>{"b", "c", "d"}));

    f.out.Clear();
    f.Run("LRANGE", {"k", "0", "100"});
    EXPECT_EQ(f.out.Last().items.size(), 4u);
}

TEST(ListCommands, LIndexAndLSet) {
    Fixture f;
    f.Run("RPUSH", {"k", "a", "b", "c"});

    f.out.Clear();
    f.Run("LINDEX", {"k", "-1"});
    EXPECT_EQ(f.out.Last().text, "c");

    f.out.Clear();
    f.Run("LSET", {"k", "1", "B"});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Ok);

    f.out.Clear();
    f.Run("LINDEX", {"k", "1"});
    EXPECT_EQ(f.out.Last().text, "B");
}

TEST(ListCommands, LInsertBeforeAfter) {
    Fixture f;
    f.Run("RPUSH", {"k", "a", "b", "d"});

    f.out.Clear();
    f.Run("LINSERT", {"k", "BEFORE", "b", "x"});
    EXPECT_EQ(f.out.Last().integer, 4);

    f.out.Clear();
    f.Run("LINSERT", {"k", "AFTER", "d", "e"});
    EXPECT_EQ(f.out.Last().integer, 5);

    f.out.Clear();
    f.Run("LRANGE", {"k", "0", "-1"});
    EXPECT_EQ(f.out.Last().items, (std::vector<std::string>{"a", "x", "b", "d", "e"}));
}
