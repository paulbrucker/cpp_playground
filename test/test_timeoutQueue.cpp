#include "ContainerQueue/TimeoutQueue.hpp"
#include <gtest/gtest.h>

TEST(TimeoutQueueTests, Instantiation)
{
    TimeoutQueue::TimeoutQueue queue{10}; // Capacity of 10
    EXPECT_EQ(queue.size(), 0);
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());
}