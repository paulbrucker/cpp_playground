#ifndef STATICQUEUE_HPP
#define STATICQUEUE_HPP

#include "Queue.hpp"
#include <array>

namespace fcp
{
template <typename T, std::size_t Capacity> class StaticQueue : public Queue
{
    using Status = Queue::Status;

  public:
    constexpr StaticQueue() = default;
    constexpr ~StaticQueue() = default;
    constexpr StaticQueue(const StaticQueue &) = default;
    constexpr StaticQueue(StaticQueue &&) = default;
    constexpr StaticQueue &operator=(const StaticQueue &) = default;
    constexpr StaticQueue &operator=(StaticQueue &&) = default;

    constexpr Status push(T const &element)
    {
        if (full())
        {
            return Status::Full;
        }

        data_[tail_] = element;
        size_++;
        tail_ = (tail_ + 1) % Capacity;

        return Status::Ok;
    }

    constexpr Status pop(T &out)
    {
        if (empty())
        {
            return Status::Empty;
        }

        out = data_[head_];
        size_--;
        head_ = (head_ + 1) % Capacity;

        return Status::Ok;
    }

    constexpr std::size_t size() const
    {
        return size_;
    }

    constexpr bool full() const
    {
        return size_ == Capacity;
    }

    constexpr bool empty() const
    {
        return size_ == 0;
    }

  private:
    std::size_t size_{0};
    std::size_t tail_{0};
    std::size_t head_{0};
    std::array<T, Capacity> data_;
};
} // namespace fcp

#endif // STATICQUEUE_HPP