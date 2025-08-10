#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <utility>

class Container
{
  public:
    constexpr explicit Container(uint16_t id) : id_{id}
    {
    }
    constexpr uint16_t GetId() const
    {
        return id_;
    }

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
    /**
     * @enum Status
     * @brief Represents the status codes returned by TimeoutQueue operations.
     *
     * The Status enum provides a set of possible outcomes for queue operations,
     * such as success, error, or special conditions.
     *
     * Values:
     * - Ok: Operation completed successfully.
     * - Full: The queue is full and cannot accept more elements.
     * - Empty: The queue is empty and there are no elements to retrieve.
     * - Null: A null pointer or invalid reference was encountered.
     * - Duplicate: An attempt was made to insert a duplicate element.
     * - NotFound: The requested element was not found in the queue.
     * - InvalidId: An invalid identifier was provided.
     */
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

    static constexpr const char *to_string(Status s) noexcept
    {
        switch (s)
        {
        case Status::Ok:
            return "Ok";
        case Status::Full:
            return "Full";
        case Status::Empty:
            return "Empty";
        case Status::Null:
            return "Null";
        case Status::Duplicate:
            return "Duplicate";
        case Status::NotFound:
            return "NotFound";
        case Status::InvalidId:
            return "InvalidId";
        default:
            return "Unknown";
        }
    }

    /**
     * @brief Constructs a TimeoutQueue with the specified capacity.
     *
     * Initializes the TimeoutQueue to hold up to the given number of elements.
     *
     * @param capacity The maximum number of elements the queue can hold.
     */
    explicit TimeoutQueue(uint16_t capacity);

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
    Status push(Container &c);

    /**
     * @brief Retrieves, but does not remove, the front element of the queue.
     *
     * @param[out] out Reference to a pointer that will be set to the front Container element.
     * @return Status indicating the result of the operation (e.g., success or failure).
     *
     * @note The caller does not take ownership of the returned pointer.
     */
    Status front(Container *&out) const;

    /**
     * @brief Removes and retrieves the next element from the queue.
     *
     * This method attempts to pop the next available element from the queue.
     * If successful, the pointer 'out' will point to the removed element.
     *
     * @param[out] out Reference to a pointer that will be set to the popped element.
     * @return Status indicating the result of the operation (e.g., success, timeout, or empty queue).
     */
    Status pop(Container *&out);

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
    Status remove(Container &c);

    std::size_t size() const;

    bool full() const;

    bool empty() const;

  private:
    static constexpr uint16_t kNil{std::numeric_limits<uint16_t>::max()};

    struct Node
    {
        uint16_t prev{kNil};
        uint16_t next{kNil};
        Container *data{nullptr};
        constexpr operator bool() const
        {
            return data != nullptr;
        }
    };

    bool IdValid(uint16_t id) const;

    void link(Node &n);

    void unlink(Node &n);

    std::size_t size_{0};
    uint16_t capacity_;
    uint16_t head_ = kNil;
    uint16_t tail_ = kNil;
    std::array<Node, kNil> data_;
};
