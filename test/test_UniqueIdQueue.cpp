#include "ContainerQueue/UniqueIdQueue.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <vector>

using namespace fcp;

namespace
{

struct TestItem
{
    uint16_t id;
    explicit TestItem(uint16_t id_) : id(id_)
    {
    }
    uint16_t GetId() const
    {
        return id;
    }
};

constexpr std::size_t kCapacity = 4;

using QueueT = UniqueIdQueue<TestItem, kCapacity>;

TEST(UniqueIdQueueTest, PushPopFrontBasic)
{
    QueueT q(kCapacity);
    TestItem a(0), b(1), c(2);

    // Initially empty
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);

    // Push one
    EXPECT_EQ(q.push(a), Queue::Status::Ok);
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1u);

    // Front returns correct pointer
    TestItem *out = nullptr;
    EXPECT_EQ(q.front(out), Queue::Status::Ok);
    EXPECT_EQ(out, &a);

    // Push another
    EXPECT_EQ(q.push(b), Queue::Status::Ok);
    EXPECT_EQ(q.size(), 2u);

    // Pop returns correct pointer
    EXPECT_EQ(q.pop(out), Queue::Status::Ok);
    EXPECT_EQ(out, &a);
    EXPECT_EQ(q.size(), 1u);

    // Front now points to b
    EXPECT_EQ(q.front(out), Queue::Status::Ok);
    EXPECT_EQ(out, &b);

    // Pop last
    EXPECT_EQ(q.pop(out), Queue::Status::Ok);
    EXPECT_EQ(out, &b);
    EXPECT_TRUE(q.empty());
}

TEST(UniqueIdQueueTest, PushDuplicate)
{
    QueueT q(kCapacity);
    TestItem a(0);

    EXPECT_EQ(q.push(a), Queue::Status::Ok);
    EXPECT_EQ(q.push(a), Queue::Status::Duplicate);
}

TEST(UniqueIdQueueTest, PushFull)
{
    QueueT q(kCapacity);
    TestItem* out = nullptr;
    std::array<TestItem, kCapacity> items = {TestItem(0), TestItem(1), TestItem(2), TestItem(3)};
    for (auto &item : items)
    {
        EXPECT_EQ(q.push(item), Queue::Status::Ok);
    }
    TestItem full(4);
    EXPECT_EQ(q.push(full), Queue::Status::Full);

    EXPECT_EQ(q.pop(out), Queue::Status::Ok);

    TestItem extra(2); // duplicate id
    EXPECT_EQ(q.push(extra), Queue::Status::Duplicate);

    TestItem over(kCapacity); // invalid id
    EXPECT_EQ(q.push(over), Queue::Status::InvalidId);

    TestItem another(1); // duplicate id
    EXPECT_EQ(q.push(another), Queue::Status::Duplicate);

    TestItem newItem(0); // duplicate id
    EXPECT_EQ(q.push(newItem), Queue::Status::Duplicate);

    // Try to push when full with a new id (but out of range)
    TestItem outOfRange(kCapacity + 1);
    EXPECT_EQ(q.push(outOfRange), Queue::Status::InvalidId);
}

TEST(UniqueIdQueueTest, RemoveByObjectAndId)
{
    QueueT q(kCapacity);
    TestItem a(0), b(1), c(2);

    EXPECT_EQ(q.push(a), Queue::Status::Ok);
    EXPECT_EQ(q.push(b), Queue::Status::Ok);
    EXPECT_EQ(q.push(c), Queue::Status::Ok);

    // Remove by object
    EXPECT_EQ(q.remove(b), Queue::Status::Ok);
    EXPECT_EQ(q.size(), 2u);

    // Remove by id
    EXPECT_EQ(q.remove(2), Queue::Status::Ok);
    EXPECT_EQ(q.size(), 1u);

    // Remove last
    EXPECT_EQ(q.remove(a), Queue::Status::Ok);
    EXPECT_TRUE(q.empty());

    // Remove from empty
    EXPECT_EQ(q.remove(a), Queue::Status::Empty);

    // Remove invalid id
    EXPECT_EQ(q.remove(uint16_t(kCapacity + 1)), Queue::Status::InvalidId);

    // Remove not found
    TestItem d(3);
    EXPECT_EQ(q.remove(d), Queue::Status::Empty);

    // Push and remove not found
    EXPECT_EQ(q.push(a), Queue::Status::Ok);
    EXPECT_EQ(q.remove(1), Queue::Status::NotFound);
}

TEST(UniqueIdQueueTest, PopEmptyAndFrontEmpty)
{
    QueueT q(kCapacity);
    TestItem *out = nullptr;
    EXPECT_EQ(q.pop(out), Queue::Status::Empty);
    EXPECT_EQ(q.front(out), Queue::Status::Empty);
}

TEST(UniqueIdQueueTest, FullAndEmpty)
{
    QueueT q(kCapacity);
    std::array<TestItem, kCapacity> items = {TestItem(0), TestItem(1), TestItem(2), TestItem(3)};
    for (auto &item : items)
    {
        EXPECT_FALSE(q.full());
        EXPECT_EQ(q.push(item), Queue::Status::Ok);
    }
    EXPECT_TRUE(q.full());
    for (std::size_t i = 0; i < kCapacity; ++i)
    {
        TestItem *out = nullptr;
        EXPECT_FALSE(q.empty());
        EXPECT_EQ(q.pop(out), Queue::Status::Ok);
    }
    EXPECT_TRUE(q.empty());
}

TEST(UniqueIdQueueTest, OrderPreserved)
{
    QueueT q(kCapacity);
    TestItem a(0), b(1), c(2), d(3);
    EXPECT_EQ(q.push(a), Queue::Status::Ok);
    EXPECT_EQ(q.push(b), Queue::Status::Ok);
    EXPECT_EQ(q.push(c), Queue::Status::Ok);
    EXPECT_EQ(q.push(d), Queue::Status::Ok);

    // Remove b (middle)
    EXPECT_EQ(q.remove(b), Queue::Status::Ok);

    // Pop should return a, then c, then d
    TestItem *out = nullptr;
    EXPECT_EQ(q.pop(out), Queue::Status::Ok);
    EXPECT_EQ(out, &a);
    EXPECT_EQ(q.pop(out), Queue::Status::Ok);
    EXPECT_EQ(out, &c);
    EXPECT_EQ(q.pop(out), Queue::Status::Ok);
    EXPECT_EQ(out, &d);
    EXPECT_TRUE(q.empty());
}

TEST(UniqueIdQueueTest, CopyAndMove)
{
    QueueT q1(kCapacity);
    TestItem a(0), b(1);
    EXPECT_EQ(q1.push(a), Queue::Status::Ok);
    EXPECT_EQ(q1.push(b), Queue::Status::Ok);

    // Copy constructor
    QueueT q2 = q1;
    EXPECT_EQ(q2.size(), 2u);

    // Move constructor
    QueueT q3 = std::move(q2);
    EXPECT_EQ(q3.size(), 2u);

    // Copy assignment
    QueueT q4(kCapacity);
    q4 = q1;
    EXPECT_EQ(q4.size(), 2u);

    // Move assignment
    QueueT q5(kCapacity);
    q5 = std::move(q4);
    EXPECT_EQ(q5.size(), 2u);
}

TEST(UniqueIdQueueTest, RemoveHeadAndTail)
{
    QueueT q(kCapacity);
    TestItem a(0), b(1), c(2);
    EXPECT_EQ(q.push(a), Queue::Status::Ok);
    EXPECT_EQ(q.push(b), Queue::Status::Ok);
    EXPECT_EQ(q.push(c), Queue::Status::Ok);

    // Remove head
    EXPECT_EQ(q.remove(a), Queue::Status::Ok);
    EXPECT_EQ(q.size(), 2u);

    // Remove tail
    EXPECT_EQ(q.remove(c), Queue::Status::Ok);
    EXPECT_EQ(q.size(), 1u);

    // Only b remains
    TestItem *out = nullptr;
    EXPECT_EQ(q.front(out), Queue::Status::Ok);
    EXPECT_EQ(out, &b);
}

} // namespace