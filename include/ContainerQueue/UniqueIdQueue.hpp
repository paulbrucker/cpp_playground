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
 *
 * @tparam T               The type of elements stored in the queue. Must provide `uint16_t GetId() const`.
 */
template <typename T, std::size_t Capacity> class UniqueIdQueue : public Queue
{
    static_assert(std::is_same<decltype(std::declval<const T &>().GetId()), uint16_t>::value,
                  "UniqueIdQueue requires T to have a method `uint16_t GetId() const`");

    using Status = Queue::Status;

    constexpr uint16_t GetId(T const &c)
    {
        return c->GetId();
    }

  public:
    explicit constexpr UniqueIdQueue(uint16_t capacity) : capacity_{capacity}
    {
    }

    constexpr UniqueIdQueue(const UniqueIdQueue &) = default;
    constexpr UniqueIdQueue &operator=(const UniqueIdQueue &) = default;

    constexpr UniqueIdQueue(UniqueIdQueue &&) = default;
    constexpr UniqueIdQueue &operator=(UniqueIdQueue &&) = default;

    constexpr Status push(T &c)
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

        node.data = &c;
        link(node);
        size_++;

        return Status::Ok;
    }

    constexpr Status front(T *&out) const
    {

        if (empty())
        {
            return Status::Empty;
        }

        out = data_[head_].data;

        return Status::Ok;
    }

    constexpr Status pop(T *&out)
    {
        if (empty())
        {
            return Status::Empty;
        }

        out = data_[head_].data;
        unlink(data_[head_]);
        size_--;

        return Status::Ok;
    }

    constexpr Status remove(T &c)
    {

        if (empty())
        {
            return Status::Empty;
        }

        auto const id = c.GetId();
        return remove(id);
    }

    constexpr Status remove(uint16_t id)
    {
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

    constexpr std::size_t size() const
    {
        return size_;
    }

    constexpr bool full() const
    {
        return size_ == capacity_;
    }

    constexpr bool empty() const
    {
        return size_ == 0;
    }

  private:
    static constexpr std::size_t kNil{std::numeric_limits<std::size_t>::max()};

    struct Node
    {
        std::size_t prev{kNil};
        std::size_t next{kNil};
        T *data{nullptr};
        constexpr operator bool() const
        {
            return data != nullptr;
        }
    };

    constexpr bool IdValid(uint16_t id) const
    {
        return id < capacity_;
    }

    constexpr void link(Node &n)
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

    constexpr void unlink(Node &n)
    {
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
    std::size_t head_ = kNil;
    std::size_t tail_ = kNil;
    std::array<Node, Capacity> data_;
};

} // namespace fcp

#endif