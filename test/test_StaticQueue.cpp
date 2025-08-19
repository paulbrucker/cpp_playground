#include "ContainerQueue/StaticQueue.hpp"
#include <gtest/gtest.h>
#include <string>

using fcp::StaticQueue;
using Status = fcp::Queue::Status;

TEST(StaticQueueTest, PushPopBasic)
{
    StaticQueue<int, 3> q;
    int out = 0;

    EXPECT_TRUE(q.empty());
    EXPECT_FALSE(q.full());
    EXPECT_EQ(q.size(), 0u);

    EXPECT_EQ(q.push(1), Status::Ok);
    EXPECT_EQ(q.size(), 1u);
    EXPECT_FALSE(q.empty());

    EXPECT_EQ(q.push(2), Status::Ok);
    EXPECT_EQ(q.push(3), Status::Ok);
    EXPECT_TRUE(q.full());
    EXPECT_EQ(q.size(), 3u);

    // Queue is full now
    EXPECT_EQ(q.push(4), Status::Full);

    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 1);
    EXPECT_EQ(q.size(), 2u);

    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 2);
    EXPECT_EQ(q.size(), 1u);

    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 3);
    EXPECT_TRUE(q.empty());
}

TEST(StaticQueueTest, PopEmpty)
{
    StaticQueue<int, 2> q;
    int out = 0;
    EXPECT_EQ(q.pop(out), Status::Empty);
}

TEST(StaticQueueTest, WrapAround)
{
    StaticQueue<int, 2> q;
    int out = 0;
    EXPECT_EQ(q.push(10), Status::Ok);
    EXPECT_EQ(q.push(20), Status::Ok);
    EXPECT_TRUE(q.full());

    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 10);

    EXPECT_EQ(q.push(30), Status::Ok);
    EXPECT_TRUE(q.full());

    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 20);

    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 30);

    EXPECT_TRUE(q.empty());
}

TEST(StaticQueueTest, Types)
{
    StaticQueue<std::string, 2> q;
    std::string out;
    EXPECT_EQ(q.push("hello"), Status::Ok);
    EXPECT_EQ(q.push("world"), Status::Ok);
    EXPECT_TRUE(q.full());
    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, "hello");
    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, "world");
}

TEST(StaticQueueTest, SizeAndCapacity)
{
    StaticQueue<int, 1> q;
    EXPECT_EQ(q.size(), 0u);
    EXPECT_EQ(q.push(42), Status::Ok);
    EXPECT_EQ(q.size(), 1u);
    int out;
    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(q.size(), 0u);
    EXPECT_TRUE(q.empty());
}

TEST(StaticQueueTest, PushPopAlternating)
{
    StaticQueue<int, 2> q;
    int out;
    EXPECT_EQ(q.push(1), Status::Ok);
    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 1);
    EXPECT_EQ(q.push(2), Status::Ok);
    EXPECT_EQ(q.pop(out), Status::Ok);
    EXPECT_EQ(out, 2);
    EXPECT_TRUE(q.empty());
}

TEST(StaticQueueTest, FillAndEmptyMultipleTimes)
{
    StaticQueue<int, 2> q;
    int out;
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(q.push(i), Status::Ok);
        EXPECT_EQ(q.push(i + 100), Status::Ok);
        EXPECT_TRUE(q.full());
        EXPECT_EQ(q.pop(out), Status::Ok);
        EXPECT_EQ(q.pop(out), Status::Ok);
        EXPECT_TRUE(q.empty());
    }
}

TEST(StaticQueueTest, CapacityZeroCompileTime)
{
    // This test will not compile if uncommented, as std::array<T, 0> is valid but not useful.
    // StaticQueue<int, 0> q;
    // int out;
    // EXPECT_EQ(q.push(1), Status::Full);
    // EXPECT_EQ(q.pop(out), Status::Empty);
    SUCCEED();
}

constexpr bool RunCompileTimeTests()
{
    // Compile-time tests for StaticQueue
    using fcp::StaticQueue;
    using Status = fcp::Queue::Status;

    // Test empty queue
    StaticQueue<int, 2> q1;
    if (!q1.empty() || q1.full() || q1.size() != 0) return false;

    // Push one element
    if (q1.push(5) != Status::Ok) return false;
    if (q1.empty() || q1.size() != 1) return false;

    // Push second element
    if (q1.push(6) != Status::Ok) return false;
    if (!q1.full() || q1.size() != 2) return false;

    // Try to push when full
    if (q1.push(7) != Status::Full) return false;

    // Pop one element
    int out = 0;
    if (q1.pop(out) != Status::Ok || out != 5) return false;
    if (q1.size() != 1) return false;

    // Pop second element
    if (q1.pop(out) != Status::Ok || out != 6) return false;
    if (!q1.empty()) return false;

    // Pop from empty
    if (q1.pop(out) != Status::Empty) return false;

    // Test with different type
    StaticQueue<char, 1> q2;
    if (q2.push('a') != Status::Ok) return false;
    char c;
    if (q2.pop(c) != Status::Ok || c != 'a') return false;

    // Test wrap-around
    StaticQueue<int, 2> q3;
    if (q3.push(1) != Status::Ok) return false;
    if (q3.push(2) != Status::Ok) return false;
    if (q3.pop(out) != Status::Ok || out != 1) return false;
    if (q3.push(3) != Status::Ok) return false;
    if (q3.pop(out) != Status::Ok || out != 2) return false;
    if (q3.pop(out) != Status::Ok || out != 3) return false;
    if (!q3.empty()) return false;

    // Test size and capacity
    StaticQueue<int, 4> q4;
    if (q4.size() != 0) return false;
    if (q4.push(1) != Status::Ok) return false;
    if (q4.push(2) != Status::Ok) return false;
    if (q4.size() != 2) return false;

    // Test alternating push/pop
    StaticQueue<int, 2> q5;
    if (q5.push(10) != Status::Ok) return false;
    if (q5.pop(out) != Status::Ok || out != 10) return false;
    if (q5.push(20) != Status::Ok) return false;
    if (q5.pop(out) != Status::Ok || out != 20) return false;
    if (!q5.empty()) return false;

    // All tests passed
    return true;
}

static_assert(RunCompileTimeTests());

