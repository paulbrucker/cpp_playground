#include "ContainerQueue/TimeoutQueue.hpp"

namespace TimeoutQueue 
{

Status TimeoutQueue::push(Container& c) 
{
    if(isFull()) {return Status::Full;}
    auto const id = c.GetId();
    if(!IdValid(id)) {return Status::InvalidId;}
    auto& node = data_[id];
    if(node) {return Status::Duplicate;}

    // Fill node with data
    node.data = &c;

    // Link the next and prev
    link(node);
    size_++;
    return Status::Ok;
}

    Status front(Container*& out) const {
        if(out == nullptr) {return Status::Null;}
        if(isEmpty()) {return Status::Empty;}
        out = data_[head_].data;
        return Status::Ok;
    }

    constexpr Status pop(Container*& out) {
        auto const result = front(out);
        if(result != Status::Ok) {return result;}
        unlink(data_[head_]);
        size_--;
        return Status::Ok;
    }

    constexpr Status remove(Container& c) {
        if(isEmpty()) {return Status::Empty;}
        auto const id = c.GetId();
        if(!IdValid(id)) {return Status::InvalidId;}
        auto& node = data_[id];
        if(!node){return Status::NotFound;}
        unlink(node);
        size_--;
        return Status::Ok;
    }

    constexpr std::size_t size() const {return size_;}
    constexpr bool isFull() const {return size_ == Capacity;}
    constexpr bool isEmpty() const {return size_ == 0;}

private:
    
    static constexpr uint16_t kNil{0xFFFF};
    struct Node {
        uint16_t prev{kNil};
        uint16_t next{kNil};
        Container* data{nullptr};
        constexpr operator bool() const { return data != nullptr;}
    };

    constexpr bool IdValid(uint16_t id) const {
        return id < Capacity;
    }

    constexpr void link(Node& n) {
        auto const id = n.data->GetId();
        n.prev = tail_;
        n.next = kNil;
        if (tail_ != kNil) {
            data_[tail_].next = id; 
        } else { head_ = id; }
        tail_ = id;
    }

    constexpr void unlink(Node& n) {
        // auto const id = n.data->GetId();
        if (n.prev != kNil) {
            data_[n.prev].next = n.next;
        } else {
            head_ = n.next;
        }
        if (n.next != kNil) {
            data_[n.next].prev = n.prev;
        } else {
            tail_ = n.prev;
        }
        n.prev = kNil;
        n.next = kNil;
        n.data = nullptr;
    }
   
}