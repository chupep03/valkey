#include "storage/Entry.hpp"

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace Storage;

TEST(Entry, EmptyNotExpired) {
    Entry e;
    EXPECT_FALSE(e.IsExpired());
}

TEST(Entry, ExpiredAfterDeadline) {
    auto past = std::chrono::system_clock::now() - std::chrono::seconds(1);
    Entry e{StringType{"x"}, past};
    EXPECT_TRUE(e.IsExpired());
}

TEST(Entry, NotExpiredInFuture) {
    auto future = std::chrono::system_clock::now() + std::chrono::seconds(60);
    Entry e{StringType{"x"}, future};
    EXPECT_FALSE(e.IsExpired());
}

TEST(Entry, MemoryGrowsWithContent) {
    Entry small{StringType{"a"}};
    Entry big{StringType{std::string(1000, 'a')}};
    EXPECT_TRUE(small.GetMemoryUsage() < big.GetMemoryUsage());
}

TEST(Entry, WithValueKeepsTtl) {
    auto future = std::chrono::system_clock::now() + std::chrono::seconds(60);
    Entry e{StringType{"old"}, future};
    Entry e2 = e.WithValue(StringType{"new"});

    EXPECT_TRUE(e2.exp_time.has_value());
    EXPECT_EQ(e2.exp_time.value(), future);
    EXPECT_EQ(std::get<StringType>(e2.value), "new");
}