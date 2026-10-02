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

TEST(GeoCommands, AddAndUpdateMember) {
    Fixture f;
    f.Run("GEOADD", {"c", "30.0", "60.0", "p"});
    EXPECT_EQ(f.out.Last().integer, 1);

    f.out.Clear();
    f.Run("GEOADD", {"c", "31.0", "61.0", "p"});
    EXPECT_EQ(f.out.Last().integer, 0);

    f.out.Clear();
    f.Run("GEOPOS", {"c", "p"});
    ASSERT_EQ(f.out.Last().items.size(), 1u);
    EXPECT_NE(f.out.Last().items[0].find("31"), std::string::npos);
}

TEST(GeoCommands, GeoPosMissingMember) {
    Fixture f;
    f.Run("GEOADD", {"c", "30", "60", "a"});
    f.out.Clear();
    f.Run("GEOPOS", {"c", "a", "missing"});
    ASSERT_EQ(f.out.Last().items.size(), 2u);
    EXPECT_EQ(f.out.Last().items[1], "(nil)");
}

TEST(GeoCommands, GeoDistSameAndDifferent) {
    Fixture f;
    f.Run("GEOADD", {"c", "30", "60", "p1"});
    f.Run("GEOADD", {"c", "30", "60", "p2"});
    f.Run("GEOADD", {"c", "31", "61", "p3"});

    f.out.Clear();
    f.Run("GEODIST", {"c", "p1", "p2", "km"});
    EXPECT_EQ(f.out.Last().text, "0.0000");

    f.out.Clear();
    f.Run("GEODIST", {"c", "p1", "p3", "km"});
    EXPECT_NE(f.out.Last().text, "0.0000");

    f.out.Clear();
    f.Run("GEODIST", {"c", "p1", "nope", "km"});
    EXPECT_EQ(f.out.Last().kind, Tests::FakeResponse::Kind::Nil);
}

TEST(GeoCommands, GeoSearchRadius) {
    Fixture f;
    f.Run("GEOADD", {"c", "30", "60", "near"});
    f.Run("GEOADD", {"c", "31", "61", "mid"});
    f.Run("GEOADD", {"c", "50", "70", "far"});

    f.out.Clear();
    f.Run("GEOSEARCH", {"c", "FROMLONLAT", "30", "60",
                        "BYRADIUS", "10", "km", "ASC"});
    EXPECT_EQ(f.out.Last().items, (std::vector<std::string>{"near"}));

    f.out.Clear();
    f.Run("GEOSEARCH", {"c", "FROMLONLAT", "30", "60",
                        "BYRADIUS", "3000", "km", "ASC"});
    ASSERT_EQ(f.out.Last().items.size(), 3u);
    EXPECT_EQ(f.out.Last().items.front(), "near");
    EXPECT_EQ(f.out.Last().items.back(), "far");

    f.out.Clear();
    f.Run("GEOSEARCH", {"c", "FROMLONLAT", "30", "60",
                        "BYRADIUS", "3000", "km", "ASC", "COUNT", "2"});
    EXPECT_EQ(f.out.Last().items.size(), 2u);
}

TEST(GeoCommands, GeoSearchStore) {
    Fixture f;
    f.Run("GEOADD", {"c", "30", "60", "a"});
    f.Run("GEOADD", {"c", "31", "61", "b"});
    f.Run("GEOADD", {"c", "50", "70", "far"});

    f.out.Clear();
    f.Run("GEOSEARCHSTORE", {"dest", "c",
                             "FROMLONLAT", "30", "60",
                             "BYRADIUS", "500", "km", "ASC"});
    EXPECT_EQ(f.out.Last().integer, 2);

    f.out.Clear();
    f.Run("SCARD", {"dest"});
    EXPECT_EQ(f.out.Last().integer, 2);

    f.out.Clear();
    f.Run("SISMEMBER", {"dest", "a"});
    EXPECT_EQ(f.out.Last().integer, 1);

    f.out.Clear();
    f.Run("SISMEMBER", {"dest", "far"});
    EXPECT_EQ(f.out.Last().integer, 0);
}