#include "ContainerQueue/TimeoutQueue.hpp"

TimeoutQueue::TimeoutQueue(uint16_t capacity) : capacity_{capacity}
{
}

TimeoutQueue::Status TimeoutQueue::push(Container &c)
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

TimeoutQueue::Status TimeoutQueue::front(Container *&out) const
{
    if (empty())
    {
        return Status::Empty;
    }

    out = data_[head_].data;
    return Status::Ok;
}

TimeoutQueue::Status TimeoutQueue::pop(Container *&out)
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

TimeoutQueue::Status TimeoutQueue::remove(Container &c)
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

std::size_t TimeoutQueue::size() const
{
    return size_;
}

bool TimeoutQueue::full() const
{
    return size_ == capacity_;
}

bool TimeoutQueue::empty() const
{
    return size_ == 0;
}

bool TimeoutQueue::IdValid(uint16_t id) const
{
    return id < capacity_;
}

void TimeoutQueue::link(Node &n)
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

void TimeoutQueue::unlink(Node &n)
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