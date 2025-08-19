#ifndef TIMEOUT_QUEUE_HPP
#define TIMEOUT_QUEUE_HPP

#include "Queue.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <utility>

namespace fcp
{

/**
 * @brief A fixed-capacity, thread-safe queue with O(1) random removal and no duplicate elements.
 *
 * The TimeoutQueue class provides a queue-like container with the following features:
 * - No duplicate elements: Each element is uniquely identified by its `GetId()` method, and only one instance of each
 * ID can exist in the queue at a time.
 * - O(1) random removal: Elements can be removed from any position in the queue in constant time using their ID.
 * - No dynamic memory allocation: All storage is preallocated based on the specified capacity.
 * - Thread safety: All operations are protected by user-supplied mutex and semaphore types/traits.
 * - Supports blocking pop operations with optional timeout.
 *
 * @tparam T               The type of elements stored in the queue. Must provide `uint16_t GetId() const`.
 * @tparam TMutex          Mutex type for thread safety.
 * @tparam TSemaphore      Semaphore type for blocking operations.
 * @tparam TMutexTrait     Trait class providing static lock/unlock/init for TMutex.
 * @tparam TSemaphoreTrait Trait class providing static wait/notify/init for TSemaphore.
 *
 * Typical use cases include resource pools, task queues, or any scenario where fast, thread-safe, duplicate-free
 * queueing with random removal is required.
 */
template <typename T, typename TMutex, typename TSemaphore, typename TMutexTrait, typename TSemaphoreTrait>
class TimeoutQueue : public Queue
{
    static_assert(std::is_same<decltype(std::declval<const T &>().GetId()), uint16_t>::value,
                  "TimeoutQueue requires T to have a method `uint16_t GetId() const`");

    using Status = Queue::Status;

  public:
    /**
     * @brief Constructs a TimeoutQueue with the specified capacity.
     *
     * Initializes the TimeoutQueue to hold up to the given number of elements.
     *
     * @param capacity The maximum number of elements the queue can hold.
     */
    explicit TimeoutQueue(uint16_t capacity) : capacity_{capacity}
    {
        TMutexTrait::init(mutex_);
        TSemaphoreTrait::init(semaphore_);
    }

    TimeoutQueue(const TimeoutQueue &) = delete;
    TimeoutQueue &operator=(const TimeoutQueue &) = delete;

    TimeoutQueue(TimeoutQueue &&) = delete;
    TimeoutQueue &operator=(TimeoutQueue &&) = delete;

    /**
     * @brief Pushes a container into the queue.
     *
     * This method attempts to add the provided container to the queue.
     * The behavior on failure or timeout depends on the implementation.
     *
     * @param c Reference to the container to be pushed into the queue.
     * @return Status indicating the result of the push operation.
     *         - Status::Duplicate if the container is already in the queue.
     *         - Status::Full if the queue is full.
     */
    Status push(T &c)
    {
        TMutexTrait::lock(mutex_);

        if (full_impl())
        {
            TMutexTrait::unlock(mutex_);
            return Status::Full;
        }

        auto const id = c.GetId();

        if (!IdValid(id))
        {
            TMutexTrait::unlock(mutex_);
            return Status::InvalidId;
        }

        auto &node = data_[id];

        if (node)
        {
            TMutexTrait::unlock(mutex_);
            return Status::Duplicate;
        }

        // Fill node with data
        node.data = &c;

        // Link the next and prev
        link(node);
        size_++;

        TMutexTrait::unlock(mutex_);
        TSemaphoreTrait::notify(semaphore_);

        return Status::Ok;
    }

    /**
     * @brief Retrieves, but does not remove, the front element of the queue.
     *
     * @param[out] out Reference to a pointer that will be set to the front Container element.
     * @return Status indicating the result of the operation (e.g., success or failure).
     *
     * @note The caller does not take ownership of the returned pointer.
     */
    Status front(T *&out) const
    {
        TMutexTrait::lock(mutex_);

        if (empty_impl())
        {
            TMutexTrait::unlock(mutex_);
            return Status::Empty;
        }

        out = data_[head_].data;

        TMutexTrait::unlock(mutex_);

        return Status::Ok;
    }

    /**
     * @brief Removes and retrieves the next element from the queue.
     *
     * This method attempts to pop the next available element from the queue.
     * If successful, the pointer 'out' will point to the removed element.
     *
     * @param[out] out Reference to a pointer that will be set to the popped element.
     * @return Status indicating the result of the operation (e.g., success, timeout, or empty queue).
     */
    Status pop(T *&out, long timeout_us = 0)
    {

        // Wait for an element to be available (this handles the mutex locking internally)
        auto pred = [this] { return !empty_impl(); };
        bool const ok = TSemaphoreTrait::wait(semaphore_, mutex_, pred, timeout_us);
        if (!ok)
        {
            return Status::Empty;
        }

        // Get the front element and remove it
        out = data_[head_].data;
        unlink(data_[head_]);
        size_--;

        TMutexTrait::unlock(mutex_);

        return Status::Ok;
    }

    /**
     * @brief Removes the specified container from the queue.
     *
     * This function attempts to remove the given container `c` from the queue,
     * regardless of its position. If the container is found, it is removed and
     * the function returns Status::Ok. If the container is not found, Status::NotFound
     * is returned.
     *
     * @param c Reference to the container to be removed from the queue.
     * @return Status indicating the result of the removal operation.
     */
    Status remove(T &c)
    {
        TMutexTrait::lock(mutex_);

        if (empty_impl())
        {
            TMutexTrait::unlock(mutex_);
            return Status::Empty;
        }

        auto const id = c.GetId();
        if (!IdValid(id))
        {
            TMutexTrait::unlock(mutex_);
            return Status::InvalidId;
        }
        auto &node = data_[id];
        if (!node)
        {
            TMutexTrait::unlock(mutex_);
            return Status::NotFound;
        }
        unlink(node);
        size_--;

        TMutexTrait::unlock(mutex_);

        return Status::Ok;
    }

    std::size_t size() const
    {
        TMutexTrait::lock(mutex_);
        auto const size{size_impl()};
        TMutexTrait::unlock(mutex_);
        return size;
    }

    bool full() const
    {
        return size() == capacity_;
    }

    bool empty() const
    {
        return size() == 0;
    }

  private:
    std::size_t size_impl() const
    {
        return size_;
    }
    bool full_impl() const
    {
        return size_impl() == capacity_;
    }

    bool empty_impl() const
    {
        return size_impl() == 0;
    }

    static constexpr uint16_t kNil{std::numeric_limits<uint16_t>::max()};

    struct Node
    {
        uint16_t prev{kNil};
        uint16_t next{kNil};
        T *data{nullptr};
        constexpr operator bool() const
        {
            return data != nullptr;
        }
    };

    bool IdValid(uint16_t id) const
    {
        return id < capacity_;
    }

    void link(Node &n)
    {
        auto const id = n.data->GetId();
        n.prev = tail_;
        n.next = kNil;
        if (tail_ != kNil)
        {
            data_[tail_].next = id;
        }
        else
        {
            head_ = id;
        }
        tail_ = id;
    }

    void unlink(Node &n)
    {
        // auto const id = n.data->GetId();
        if (n.prev != kNil)
        {
            data_[n.prev].next = n.next;
        }
        else
        {
            head_ = n.next;
        }
        if (n.next != kNil)
        {
            data_[n.next].prev = n.prev;
        }
        else
        {
            tail_ = n.prev;
        }
        n.prev = kNil;
        n.next = kNil;
        n.data = nullptr;
    }

    std::size_t size_{0};
    uint16_t capacity_;
    uint16_t head_ = kNil;
    uint16_t tail_ = kNil;
    std::array<Node, kNil> data_;

    mutable TMutex mutex_;
    TSemaphore semaphore_;
};

} // namespace fcp

#endif