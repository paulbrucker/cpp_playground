#ifndef QUEUE_HPP
#define QUEUE_HPP

namespace fcp
{

class Queue
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
        InvalidId = -6,
        Timeout = -7
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
};

} // namespace fcp

#endif