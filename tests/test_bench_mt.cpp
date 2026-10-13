#include "commands/CommandReg.hpp"
#include "commands/RegisterAll.hpp"
#include "storage/MemoryManager.hpp"
#include "storage/StorageEngine.hpp"
#include "fake_response.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

using namespace Commands;
using Storage::MemoryManager;
using Storage::StorageEngine;

namespace {

struct Worker {
    StorageEngine& engine;
    MemoryManager& mm;
    CommandReg&    reg;
    Tests::FakeResponse out;

    Worker(StorageEngine& e, MemoryManager& m, CommandReg& r)
        : engine(e), mm(m), reg(r) {}

    void Run(const std::string& name, std::vector<std::string> args) {
        auto* cmd = reg.FindCommand(name);
        ASSERT_NE(cmd, nullptr);
        Context ctx(name, std::move(args), engine, mm, out);
        cmd->Execute(ctx);
    }
};

} // namespace

TEST(StressMT, SAddSameKey) {
    MemoryManager mm;
    StorageEngine engine(mm);
    CommandReg reg;
    RegisterAll(reg);

    constexpr int kThreads = 4;
    constexpr int kPerThread = 1000;

    std::vector<std::thread> workers;
    workers.reserve(kThreads);

    for (int t = 0; t < kThreads; t++) {
        workers.emplace_back([&, t]() {
            Worker w(engine, mm, reg);
            for (int i = 0; i < kPerThread; ++i) {
                std::string member = "t" + std::to_string(t) + "_" + std::to_string(i);
                w.Run("SADD", {"s", member});
            }
        });
    }

    for (auto& th : workers) th.join();

    Worker check(engine, mm, reg);
    check.Run("SCARD", {"s"});
    EXPECT_EQ(check.out.Last().integer, kThreads * kPerThread);
}

TEST(StressMT, SaddDiffKeys) {
    MemoryManager mm;
    StorageEngine engine(mm);
    CommandReg reg;
    RegisterAll(reg);

    constexpr int kThreads = 4;
    constexpr int kPerThread = 1000;

    std::vector<std::thread> workers;
    workers.reserve(kThreads);

    for (int t = 0;  t < kThreads; t++) {
        workers.emplace_back([&, t]() {
            Worker w(engine, mm, reg);
            for (int k = 0; k < kPerThread; k++) {
                std::string member = "t" + std::to_string(t) + "_" + std::to_string(k);
                std::string key = "key_" + std::to_string(t);
                w.Run("SADD", {key, member});
            }
        });
    }

    for (auto& th : workers) th.join();

    for (int t = 0; t < kThreads; t++) {
        Worker check(engine, mm, reg);
        check.Run("SCARD", {"key_" + std::to_string(t)});
        EXPECT_EQ(check.out.Last().integer, kPerThread) << "wrong cardinality for key_" << t;
    }
}

TEST(StressMT, SetGetParrallel) {
    MemoryManager mm;
    StorageEngine engine(mm);
    CommandReg reg;
    RegisterAll(reg);

    constexpr int kThreads = 4;
    constexpr int kPerThread = 1000;

    std::vector<std::thread> workers;
    workers.reserve(kThreads);

    // writers (2 threads)
    for (int t = 0; t < 2; t++) {
        workers.emplace_back([&, t]() {
            Worker w(engine, mm, reg);
            for (int k = 0; k < kPerThread; k++) {
                std::string member = "t" + std::to_string(t) + "_" + std::to_string(k);
                w.Run("SADD", {"shared_key", member});
            }
        });
    }

    // readers(2 threads)
    std::atomic<int> reader_errors = 0;
    for (int t = 0; t < 2; t++) {
        workers.emplace_back([&, t]() {
            Worker w(engine, mm, reg);
            for (int k = 0; k < kPerThread; k++) {
                w.out.Clear();
                w.Run("SCARD", {"shared_key"});

                long long count = w.out.Last().integer;
                if (count < 0 || count > 2 * kPerThread) {
                    reader_errors++;
                }

                w.out.Clear();
                w.Run("SMEMBERS", {"shared_key"});
                auto& items = w.out.Last().items;
                if (items.size() > static_cast<size_t>(kPerThread * 2)) {
                    reader_errors++;
                }
            }
        });
    }

    for (auto& th : workers) th.join();
    EXPECT_EQ(reader_errors.load(), 0);

    Worker check(engine, mm, reg);
    check.Run("SCARD", {"shared_key"});
    EXPECT_EQ(check.out.Last().integer, 2 * kPerThread);
}

TEST(StressMT, RemoveWhileReading) {
    MemoryManager mm;
    StorageEngine engine(mm);
    CommandReg reg;
    RegisterAll(reg);

    constexpr int kPerThread = 1000;
    const std::string key = "key";

    std::atomic<bool> stop{false};

    std::thread writer([&]() {
        Worker w(engine, mm, reg);
        for (int i = 0; i < kPerThread; ++i) {
            w.out.Clear();
            w.Run("SET", {key, "value_" + std::to_string(i)});
            w.out.Clear();
            w.Run("DEL", {key});
        }
        stop.store(true);
    });

    std::thread reader1([&]() {
        Worker w(engine, mm, reg);
        while (!stop.load()) {
            w.out.Clear();
            w.Run("GET", {key});
            // must be not an exception
        }
    });

    std::thread reader2([&]() {
        Worker w(engine, mm, reg);
        while (!stop.load()) {
            w.out.Clear();
            w.Run("EXISTS", {key});
        }
    });

    writer.join();
    reader1.join();
    reader2.join();

    Worker check(engine, mm, reg);
    check.Run("EXISTS", {key});
    EXPECT_EQ(check.out.Last().integer, 0);
    EXPECT_EQ(engine.Size(), 0);
}

TEST(StressMT, ExpiringKey) {
    MemoryManager mm;
    StorageEngine engine(mm);
    CommandReg reg;
    RegisterAll(reg);

    constexpr int kPerThread = 1000;
    constexpr int kTtlSeconds = 10;
    const std::string key = "key";

    std::thread writer([&]() {
        Worker w(engine, mm, reg);
        for (int i = 0; i < kPerThread; ++i) {
            w.out.Clear();
            w.Run("SET", {key, "value"});
            w.out.Clear();
            w.Run("EXPIRE", {key, std::to_string(kTtlSeconds)});
        }
    });

    std::atomic<int> reader_errors{0};
    std::thread reader([&]() {
        Worker w(engine, mm, reg);
        for (int i = 0; i < kPerThread; ++i) {
            w.out.Clear();
            w.Run("GET", {key});
            if (w.out.Last().kind != Tests::FakeResponse::Kind::Bulk) {
                reader_errors++;
            }

            w.out.Clear();
            w.Run("TTL", {key});
            long long ttl = w.out.Last().integer;
            if (ttl < -1 || ttl > kTtlSeconds) {
                reader_errors++;
            }
        }
    });

    writer.join();
    reader.join();

    EXPECT_EQ(reader_errors.load(), 0);

    Worker check(engine, mm, reg);
    check.Run("TTL", {key});
    long long ttl = check.out.Last().integer;
    EXPECT_TRUE(ttl == -1 || (ttl > 0 && ttl <= kTtlSeconds));
}

TEST(StressMT, ExpireWhileReading) {
    MemoryManager mm;
    StorageEngine engine(mm);
    CommandReg reg;
    RegisterAll(reg);

    constexpr int kCycles = 3;
    const std::string key = "key";

    std::atomic<bool> stop{false};
    std::atomic<long long> bulk_count{0};
    std::atomic<long long> nil_count{0};
    std::atomic<int> reader_errors{0};

    std::thread writer([&]() {
        Worker w(engine, mm, reg);
        for (int c = 0; c < kCycles; ++c) {
            w.out.Clear();
            w.Run("SET", {key, "value"});

            w.out.Clear();
            w.Run("EXPIRE", {key, "1"});
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        }
        stop.store(true);
    });

    std::thread reader([&]() {
        Worker w(engine, mm, reg);
        while (!stop.load()) {
            w.out.Clear();
            w.Run("GET", {key});
            auto kind = w.out.Last().kind;
            if (kind == Tests::FakeResponse::Kind::Bulk) {
                bulk_count.fetch_add(1);
            } else if (kind == Tests::FakeResponse::Kind::Nil) {
                nil_count.fetch_add(1);
            } else {
                reader_errors.fetch_add(1);
            }

            // -2 (no key), -1 (key exist but without ttl TTL)
            // 0, 1 (with ttl)
            // otherwise - bug
            w.out.Clear();
            w.Run("TTL", {key});
            long long ttl = w.out.Last().integer;
            if (ttl < -2 || ttl > 1) {
                reader_errors.fetch_add(1);
            }
        }
    });

    writer.join();
    reader.join();

    EXPECT_EQ(reader_errors.load(), 0);

    EXPECT_GT(bulk_count.load(), 0) << "reader never saw a live key";
    EXPECT_GT(nil_count.load(),  0) << "reader never saw an expired key";

    Worker check(engine, mm, reg);
    check.Run("GET", {key});
    EXPECT_EQ(check.out.Last().kind, Tests::FakeResponse::Kind::Nil);

    check.out.Clear();
    check.Run("TTL", {key});
    EXPECT_EQ(check.out.Last().integer, -2);

    check.out.Clear();
    check.Run("EXISTS", {key});
    EXPECT_EQ(check.out.Last().integer, 0);
}

TEST(StressMT, DBSizeWhileWriting) {
    MemoryManager mm;
    StorageEngine engine(mm);
    CommandReg reg;
    RegisterAll(reg);

    constexpr int kPerThread = 1000;

    std::atomic<bool> stop{false};
    std::atomic<bool> dbsize_ok{true};

    std::thread writer([&]() {
        Worker w(engine, mm, reg);
        for (int k = 0; k < kPerThread; k++) {
            std::string key = "key_" + std::to_string(k);
            w.Run("SET", {key, "member"});
        }
        
        stop.store(true);
    });

    std::thread reader([&](){
        Worker w(engine, mm, reg);
        long long prev = 0;
        while (!stop.load()) {
            w.out.Clear();
            w.Run("DBSIZE", {});
            long long cur = w.out.Last().integer;

            if (cur < prev) dbsize_ok.store(false);
            if (cur < 0 || cur > kPerThread) dbsize_ok.store(false);
            prev = cur;
        }
    });

    writer.join();
    reader.join();

    EXPECT_TRUE(dbsize_ok.load());
    
    Worker check(engine, mm, reg);
    check.Run("DBSIZE", {});
    EXPECT_EQ(check.out.Last().integer, kPerThread);
}