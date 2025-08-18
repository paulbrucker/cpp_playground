#include "ContainerQueue/TimeoutQueue.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

class MockContainer
{
  public:
    constexpr explicit MockContainer(uint16_t id) : id_{id}
    {
    }
    constexpr uint16_t GetId() const
    {
        return id_;
    }

  private:
    uint16_t id_{};
};

using Container = MockContainer;
using TimeoutQueue = fcp::TimeoutQueue<MockContainer>;
using Status = fcp::Queue::Status;

class TimeoutQueueTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        queue = std::make_unique<TimeoutQueue>(10);
    }

    void TearDown() override
    {
        queue.reset();
    }

    std::unique_ptr<TimeoutQueue> queue;
};

// Test instantiation and initial state
TEST_F(TimeoutQueueTest, Instantiation)
{
    EXPECT_EQ(queue->size(), 0);
    EXPECT_TRUE(queue->empty());
    EXPECT_FALSE(queue->full());
}

// Test constructor with different capacities
TEST(TimeoutQueueTests, ConstructorWithDifferentCapacities)
{
    TimeoutQueue small_queue(1);
    EXPECT_EQ(small_queue.size(), 0);
    EXPECT_TRUE(small_queue.empty());
    EXPECT_FALSE(small_queue.full());

    TimeoutQueue large_queue(1000);
    EXPECT_EQ(large_queue.size(), 0);
    EXPECT_TRUE(large_queue.empty());
    EXPECT_FALSE(large_queue.full());
}

// Test basic push operation
TEST_F(TimeoutQueueTest, BasicPush)
{
    Container c1(1);
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->size(), 1);
    EXPECT_FALSE(queue->empty());
    EXPECT_FALSE(queue->full());
}

// Test push with invalid ID (ID >= capacity)
TEST_F(TimeoutQueueTest, PushInvalidId)
{
    Container c_invalid(15); // ID 15 is >= capacity 10
    EXPECT_EQ(queue->push(c_invalid), Status::InvalidId);
    EXPECT_EQ(queue->size(), 0);
    EXPECT_TRUE(queue->empty());
}

// Test push duplicate
TEST_F(TimeoutQueueTest, PushDuplicate)
{
    Container c1(1);
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->push(c1), Status::Duplicate);
    EXPECT_EQ(queue->size(), 1);
}

// Test push until full
TEST_F(TimeoutQueueTest, PushUntilFull)
{
    std::vector<Container> containers;
    for (uint16_t i = 0; i < 10; ++i)
    {
        containers.emplace_back(i);
        EXPECT_EQ(queue->push(containers.back()), Status::Ok);
        EXPECT_EQ(queue->size(), i + 1);
    }

    EXPECT_TRUE(queue->full());
    EXPECT_FALSE(queue->empty());

    // Try to push one more
    Container extra(0); // This will be a duplicate anyway
    EXPECT_EQ(queue->push(extra), Status::Full);
}

// Test front operation
TEST_F(TimeoutQueueTest, Front)
{
    Container c1(1);
    Container c2(2);
    Container *out = nullptr;

    // Front on empty queue
    EXPECT_EQ(queue->front(out), Status::Empty);

    // Add elements and test front
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->front(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 1);

    // Add another element, front should still be first
    EXPECT_EQ(queue->push(c2), Status::Ok);
    EXPECT_EQ(queue->front(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 1);
}

// Test pop operation
TEST_F(TimeoutQueueTest, Pop)
{
    Container c1(1);
    Container c2(2);
    Container *out = nullptr;

    // Pop from empty queue
    EXPECT_EQ(queue->pop(out), Status::Empty);

    // Add elements and pop
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->push(c2), Status::Ok);
    EXPECT_EQ(queue->size(), 2);

    // Pop first element
    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 1);
    EXPECT_EQ(queue->size(), 1);

    // Pop second element
    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 2);
    EXPECT_EQ(queue->size(), 0);
    EXPECT_TRUE(queue->empty());

    // Pop from empty queue again
    EXPECT_EQ(queue->pop(out), Status::Empty);
}

// Test remove operation
TEST_F(TimeoutQueueTest, Remove)
{
    Container c1(1);
    Container c2(2);
    Container c3(3);

    // Remove from empty queue
    EXPECT_EQ(queue->remove(c1), Status::Empty);

    // Add elements
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->push(c2), Status::Ok);
    EXPECT_EQ(queue->push(c3), Status::Ok);
    EXPECT_EQ(queue->size(), 3);

    // Remove middle element
    EXPECT_EQ(queue->remove(c2), Status::Ok);
    EXPECT_EQ(queue->size(), 2);

    // Try to remove the same element again
    EXPECT_EQ(queue->remove(c2), Status::NotFound);

    // Remove remaining elements
    EXPECT_EQ(queue->remove(c1), Status::Ok);
    EXPECT_EQ(queue->remove(c3), Status::Ok);
    EXPECT_TRUE(queue->empty());
}

// Test remove with invalid ID
TEST_F(TimeoutQueueTest, RemoveInvalidId)
{
    Container c1(1);
    Container c_invalid(15); // ID >= capacity

    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->remove(c_invalid), Status::InvalidId);
    EXPECT_EQ(queue->size(), 1);
}

// Test remove non-existent element with valid ID
TEST_F(TimeoutQueueTest, RemoveNotFound)
{
    Container c1(1);
    Container c2(2);

    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->remove(c2), Status::NotFound);
    EXPECT_EQ(queue->size(), 1);
}

// Test FIFO order
TEST_F(TimeoutQueueTest, FIFOOrder)
{
    std::array<Container, 5> containers{
        Container(0), Container(1), Container(2), Container(3), Container(4),
    };

    Container *out = nullptr;

    for (auto &container : containers)
    {
        EXPECT_EQ(queue->push(container), Status::Ok);
    }

    // Pop them in order
    for (uint16_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(queue->pop(out), Status::Ok);
        EXPECT_EQ(out->GetId(), i);
    }

    EXPECT_TRUE(queue->empty());
}

// Test queue behavior with random removal
TEST_F(TimeoutQueueTest, RandomRemoval)
{
    Container c1(1);
    Container c2(2);
    Container c3(3);
    Container c4(4);
    Container *out = nullptr;

    // Add elements
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->push(c2), Status::Ok);
    EXPECT_EQ(queue->push(c3), Status::Ok);
    EXPECT_EQ(queue->push(c4), Status::Ok);

    // Remove middle elements
    EXPECT_EQ(queue->remove(c2), Status::Ok);
    EXPECT_EQ(queue->remove(c3), Status::Ok);

    // Pop remaining elements - should maintain order
    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 1);

    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 4);

    EXPECT_TRUE(queue->empty());
}

// Test adding element after removing it
TEST_F(TimeoutQueueTest, AddAfterRemove)
{
    Container c1(1);
    Container c2(2);
    Container *out = nullptr;

    // Add and remove
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->push(c2), Status::Ok);
    EXPECT_EQ(queue->remove(c1), Status::Ok);

    // Add the same element again
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->size(), 2);

    // Check order: c2 should be first, then c1
    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 2);

    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 1);
}

// Test edge case: single element operations
TEST_F(TimeoutQueueTest, SingleElementOperations)
{
    Container c1(1);
    Container *out = nullptr;

    // Add single element
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->size(), 1);
    EXPECT_FALSE(queue->empty());
    EXPECT_FALSE(queue->full());

    // Front should work
    EXPECT_EQ(queue->front(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 1);

    // Remove the element
    EXPECT_EQ(queue->remove(c1), Status::Ok);
    EXPECT_TRUE(queue->empty());

    // Add again and pop
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_EQ(out->GetId(), 1);
    EXPECT_TRUE(queue->empty());
}

// Test capacity boundary cases
TEST(TimeoutQueueTests, CapacityBoundary)
{
    TimeoutQueue queue(3);
    Container c0(0);
    Container c1(1);
    Container c2(2);
    Container c3(3); // Invalid ID

    // Fill to capacity
    EXPECT_EQ(queue.push(c0), Status::Ok);
    EXPECT_EQ(queue.push(c1), Status::Ok);
    EXPECT_EQ(queue.push(c2), Status::Ok);
    EXPECT_TRUE(queue.full());

    // Try invalid ID
    EXPECT_EQ(queue.push(c3), Status::Full);
}

// Test Container class functionality
TEST(ContainerTests, ContainerFunctionality)
{
    Container c1(42);
    EXPECT_EQ(c1.GetId(), 42);

    Container c2(0);
    EXPECT_EQ(c2.GetId(), 0);

    Container c3(std::numeric_limits<uint16_t>::max() - 1);
    EXPECT_EQ(c3.GetId(), std::numeric_limits<uint16_t>::max() - 1);
}

// Test queue state consistency
TEST_F(TimeoutQueueTest, StateConsistency)
{
    Container c1(1);
    Container c2(2);
    Container *out = nullptr;

    // Initially empty
    EXPECT_TRUE(queue->empty());
    EXPECT_FALSE(queue->full());
    EXPECT_EQ(queue->size(), 0);

    // Add one element
    EXPECT_EQ(queue->push(c1), Status::Ok);
    EXPECT_FALSE(queue->empty());
    EXPECT_FALSE(queue->full());
    EXPECT_EQ(queue->size(), 1);

    // Add another
    EXPECT_EQ(queue->push(c2), Status::Ok);
    EXPECT_FALSE(queue->empty());
    EXPECT_FALSE(queue->full());
    EXPECT_EQ(queue->size(), 2);

    // Remove one
    EXPECT_EQ(queue->remove(c1), Status::Ok);
    EXPECT_FALSE(queue->empty());
    EXPECT_FALSE(queue->full());
    EXPECT_EQ(queue->size(), 1);

    // Pop the last one
    EXPECT_EQ(queue->pop(out), Status::Ok);
    EXPECT_TRUE(queue->empty());
    EXPECT_FALSE(queue->full());
    EXPECT_EQ(queue->size(), 0);
}
