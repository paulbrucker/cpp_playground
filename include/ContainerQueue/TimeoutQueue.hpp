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
// Queue API
// Random deletion of elements
// No duplets in the queue
// No dynamic memory allocation
template <typename T> class TimeoutQueue : public Queue
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
        if (full())
        {
            return Status::Full;
        }

        auto const id = c.GetId();

        if (!IdValid(id))
        {
            return Status::InvalidId;
        }

        auto &node = data_[id];

        if (node)
        {
            return Status::Duplicate;
        }

        // Fill node with data
        node.data = &c;

        // Link the next and prev
        link(node);
        size_++;
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
        if (empty())
        {
            return Status::Empty;
        }

        out = data_[head_].data;
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
    Status pop(T *&out)
    {
        auto const result = front(out);
        if (result != Status::Ok)
        {
            return result;
        }
        unlink(data_[head_]);
        size_--;
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
        if (empty())
        {
            return Status::Empty;
        }
        auto const id = c.GetId();
        if (!IdValid(id))
        {
            return Status::InvalidId;
        }
        auto &node = data_[id];
        if (!node)
        {
            return Status::NotFound;
        }
        unlink(node);
        size_--;
        return Status::Ok;
    }

    std::size_t size() const
    {
        return size_;
    }

    bool full() const
    {
        return size_ == capacity_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

  private:
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
};

} // namespace fcp

#endif