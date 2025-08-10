#include <utility>
#include <array>
#include <cstdint>
#include <algorithm>

class Container {
public:
    constexpr explicit Container(uint16_t id) : id_{id} {}
    constexpr uint16_t GetId() const {return id_;}
private:
    uint16_t id_{};
};


// Queue API
// Random deletion of elements
// No duplets in the queue
// No dynamic memory allocation
class TimeoutQueue 
{
public:
    enum class Status 
    {
        Ok = 0,
        Full = -1,
        Empty = -2,
        Null = -3,
        Duplicate = -4,
        NotFound = -5,
        InvalidId = -6
    };

    static constexpr const char* to_string(Status s) noexcept {
        switch (s) {
            case Status::Ok:        return "Ok";
            case Status::Full:      return "Full";
            case Status::Empty:     return "Empty";
            case Status::Null:      return "Null";
            case Status::Duplicate: return "Duplicate";
            case Status::NotFound:  return "NotFound";
            case Status::InvalidId: return "InvalidId";
            default:                return "Unknown";
        }
    }

    explicit TimeoutQueue(uint16_t capacity);
        
    Status push(Container& c);

    Status front(Container*& out);

    Status pop(Container*& out);

    Status remove(Container& c);

    std::size_t size() const {return size_;}
    bool full() const {return size_ == capacity_;}
    bool empty() const {return size_ == 0;}

private:
    
    static constexpr uint16_t kNil{std::numeric_limits<uint16_t>::max()};
    
    struct Node {
        uint16_t prev{kNil};
        uint16_t next{kNil};
        Container* data{nullptr};
        constexpr operator bool() const { return data != nullptr;}
    };

    bool IdValid(uint16_t id) const {
        return id < capacity_;
    }

    void link(Node& n);

    void unlink(Node& n);

    std::size_t size_{0};
    uint16_t capacity_;
    uint16_t head_ = kNil;
    uint16_t tail_ = kNil;
    std::array<Node, kNil> data_;
};
