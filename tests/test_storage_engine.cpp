#include "storage/MemoryManager.hpp"
#include "storage/StorageEngine.hpp"

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace Storage;

namespace {
Entry MakeString(std::string v) {
    return Entry{StringType{std::move(v)}};
}
}

TEST(StorageEngine, SetGet) {
    MemoryManager mm;
    StorageEngine e(mm);

    e.Set("key", MakeString("hello"));
    auto got = e.Get("key");
    ASSERT_TRUE(got);
    EXPECT_EQ(std::get<StringType>(got->value), "hello");
}

TEST(StorageEngine, GetMissingReturnsNull) {
    MemoryManager mm;
    StorageEngine e(mm);
    EXPECT_FALSE(e.Get("nope"));
}

TEST(StorageEngine, SetReplaces) {
    MemoryManager mm;
    StorageEngine e(mm);
    e.Set("key", MakeString("a"));
    e.Set("key", MakeString("b"));
    EXPECT_EQ(std::get<StringType>(e.Get("key")->value), "b");
    EXPECT_EQ(e.Size(), 1u);
}

TEST(StorageEngine, RemoveCounts) {
    MemoryManager mm;
    StorageEngine e(mm);
    e.Set("key1", MakeString("1"));
    e.Set("key2", MakeString("2"));
    EXPECT_EQ(e.Remove({"key1", "key2", "extra_key"}), 2);
    EXPECT_EQ(e.Size(), 0u);
}

TEST(StorageEngine, ExpiredKeyInvisible) {
    MemoryManager mm;
    StorageEngine e(mm);
    auto past = std::chrono::system_clock::now() - std::chrono::seconds(1);
    e.Set("k", Entry{StringType{"x"}, past});

    EXPECT_FALSE(e.Get("k"));
    EXPECT_FALSE(e.Exist("k"));
    EXPECT_EQ(e.Size(), 0u);
    EXPECT_EQ(mm.GetUsage(), 0u);
}

TEST(StorageEngine, CoWKeepsOldSharedPtrAlive) {
    MemoryManager mm;
    StorageEngine e(mm);
    e.Set("key", MakeString("old"));
    auto old = e.Get("key");

    e.Set("key", MakeString("new"));
    auto now = e.Get("key");

    ASSERT_TRUE(old);
    EXPECT_EQ(std::get<StringType>(old->value), "old");
    EXPECT_EQ(std::get<StringType>(now->value), "new");
}

TEST(StorageEngine, MemoryAccountingRoundTrip) {
    MemoryManager mm;
    StorageEngine e(mm);
    e.Set("key1", MakeString("hello"));
    e.Set("key2", MakeString("world"));
    EXPECT_GT(mm.GetUsage(), 0u);

    e.Remove({"key1", "key2"});
    EXPECT_EQ(mm.GetUsage(), 0u);
}

TEST(StorageEngine, FlushResetsUsage) {
    MemoryManager mm;
    StorageEngine e(mm);
    e.Set("a", MakeString("hello"));
    e.Set("b", MakeString("world"));
    e.Flush();
    EXPECT_EQ(e.Size(), 0u);
    EXPECT_EQ(mm.GetUsage(), 0u);
}

TEST(StorageEngine, KeysGlob) {
    MemoryManager mm;
    StorageEngine e(mm);
    e.Set("user1", MakeString("a"));
    e.Set("user2", MakeString("b"));
    e.Set("post1", MakeString("c"));

    auto keys = e.GetAllKeysByPattern("user*");
    EXPECT_EQ(keys.size(), 2u);

    keys = e.GetAllKeysByPattern("?1"); 
    keys = e.GetAllKeysByPattern("user?");
    EXPECT_EQ(keys.size(), 2u);
}