#include "storage/MemoryManager.hpp"
#include "storage/StorageException.hpp"

#include <gtest/gtest.h>

using Storage::MemoryManager;

TEST(MemoryManager, ParsePlainBytes) {
    EXPECT_EQ(MemoryManager::ParseSizeToBytes("0"), 0u);
    EXPECT_EQ(MemoryManager::ParseSizeToBytes("1024"), 1024u);
    EXPECT_EQ(MemoryManager::ParseSizeToBytes("512b"), 512u);
}

TEST(MemoryManager, ParseSuffixesCaseInsensitive) {
    EXPECT_EQ(MemoryManager::ParseSizeToBytes("1kb"), 1024u);
    EXPECT_EQ(MemoryManager::ParseSizeToBytes("1KB"), 1024u);
    EXPECT_EQ(MemoryManager::ParseSizeToBytes("64mb"), 64u * 1024u * 1024u);
    EXPECT_EQ(MemoryManager::ParseSizeToBytes("2gb"), 2u * 1024u * 1024u * 1024u);
}

TEST(MemoryManager, ParseRejectsGarbage) {
    EXPECT_THROW(MemoryManager::ParseSizeToBytes(""), std::runtime_error);
    EXPECT_THROW(MemoryManager::ParseSizeToBytes("abc"), std::runtime_error);
    EXPECT_THROW(MemoryManager::ParseSizeToBytes("12xb"), std::runtime_error);
    EXPECT_THROW(MemoryManager::ParseSizeToBytes("mb"), std::runtime_error);
}

TEST(MemoryManager, NoLimitByDefault) {
    MemoryManager mm;
    EXPECT_EQ(mm.GetLimit(), 0u);
    EXPECT_TRUE(mm.CanAllocate(1'000'000, 0));
}

TEST(MemoryManager, ResizeRespectsLimit) {
    MemoryManager mm(1024);
    mm.Resize(0, 512);
    EXPECT_EQ(mm.GetUsage(), 512u);

    mm.Resize(512, 1024);
    EXPECT_EQ(mm.GetUsage(), 1024u);

    EXPECT_THROW(mm.Resize(1024, 2048), Storage::OutOfMemoryException);
    EXPECT_EQ(mm.GetUsage(), 1024u);
}

TEST(MemoryManager, ResizeShrink) {
    MemoryManager mm(1024);
    mm.Resize(0, 800);
    mm.Resize(800, 100);
    EXPECT_EQ(mm.GetUsage(), 100u);
}

TEST(MemoryManager, SetLimitBelowUsageThrows) {
    MemoryManager mm(1024);
    mm.Resize(0, 512);
    EXPECT_THROW(mm.SetLimit(256), std::invalid_argument);
    EXPECT_NO_THROW(mm.SetLimit(512));
    EXPECT_NO_THROW(mm.SetLimit(0));
}