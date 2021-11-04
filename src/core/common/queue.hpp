#include <atomic>
#include <thread>
#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <stdexcept>
// a thread-safe queue with a maximal size based on std::list<>::splice()
template <typename T>
class ThreadSafeQ
{
private:
    // check whether the Q is running or closed
    enum class State
    {
        OPEN,
        CLOSED
    };
    State state; // The current state
    size_t currSize; // The current size
    size_t maxSize; // The maximal size
    std::condition_variable cvPush, cvPop; // The condition variables to use for pushing/popping
    std::mutex mutex; // The mutex for locking the Q
    std::list<T> list; // The list that the Q is implemented with
public:
    // enum to return to check whether could push or pop or queue was closed
    enum QueueResult
    {
        OK,
        CLOSED
    };
    // create and initialize the Q with maximum size
    explicit ThreadSafeQ(size_t maxSize = 0) : state(State::OPEN), currSize(0), maxSize(maxSize)
    {}
    // Push data to Q, if queue is full then  blocks
    void push(T const& data)
    {
        // Creating temporary Q
        decltype(list) tmpl;
        tmpl.push_back(data);
        // Push with lock
        {
            std::unique_lock<std::mutex> lock(mutex);
            // wait until space is there in the Q
            while (currSize == maxSize)
                cvPush.wait(lock);
            // Check whether the Q is closed or not and pushing is allowed
            if (state == State::CLOSED)
                throw std::runtime_error("The queue is closed and trying to push.");
            // Pushing to Q
            currSize += 1;
            list.splice(list.end(), tmpl, tmpl.begin());
            // popping thread to wake up
            if (currSize == 1u)
                cvPop.notify_one();
        }
    }
    // Push data to Q with rvalue reference
    void push(T&& data)
    {
        // Create temporary queue.
        decltype(list) tmpl;
        tmpl.push_back(data);
        // Push with lock
        {
            std::unique_lock<std::mutex> lock(mutex);
            // wait until space is there in the Q
            while (currSize == maxSize)
                cvPush.wait(lock);
            // Check whether the Q is closed or not and pushing is allowed
            if (state == State::CLOSED)
                throw std::runtime_error("The queue is closed and trying to push.");
            // Pushing to Q
            currSize += 1;
            list.splice(list.end(), tmpl, tmpl.begin());
            // popping thread to wake up
            cvPop.notify_one();
        }
    }
    // Poping value from Q and write to var
    // If successful, OK is returned, else if the Q is empty and was closed, then CLOSED is returned
    QueueResult pop(T& var)
    {
        decltype(list) tmpl;
        // Pop data to the tmpl
        {
            std::unique_lock<std::mutex> lock(mutex);
            // wait until there is data, if there is no data
            while (list.empty() && state != State::CLOSED)
                cvPop.wait(lock);
            // cannot return anything, if the Q was closed and the list is empty
            if (list.empty() && state == State::CLOSED)
                return CLOSED;
            // If data found
            currSize -= 1;
            tmpl.splice(tmpl.begin(), list, list.begin());
            // one pushing thread wake up
            cvPush.notify_one();
        }
        // data write to var
        var = tmpl.front();
        return OK;
    }
    // No pushing data when the queue is closed
    void close() noexcept
    {
        std::unique_lock<std::mutex> lock(mutex);
        state = State::CLOSED;
        // all consumers notify
        cvPop.notify_all();
    }
};