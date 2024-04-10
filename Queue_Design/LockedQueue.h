#ifndef  _LOCKEDQUEUE_H
#define  _LOCKEDQUEUE_H

//Locked Queue by C++
#include <mutex>
#include <deque>
#include <atomic>

template <class T, typename StorageType = std::deque<T>>
class LockedQueue {
private:
    // lock access to the queue
    std::mutex _lock;

    // storage backing the queue
    StorageType _queue;

    // cancellation flag
    volatile bool _canceled;


public:

    // create a locked queue
    LockedQueue(): _canceled(false) {}

    // destroy a locked queue
    ~LockedQueue() {}

    // lock the queue for access
    void lock() {
        this->_lock.lock();
    }

    // unlock the queue
    void unlock() {
        this->_lock.unlock();
    }

    // pop_front of the queue
    void pop_front() {
        std::lock_guard<std::mutex> lock(_lock);
        _queue.pop_front();
    }

    // check if the queue is empty
    bool empty() {
        std::lock_guard<std::mutex> lock(_lock);
        return _queue.empty();
    }

    // cancel the queue
    void cancel() {
        std::lock_guard<std::mutex> lock(_lock);
        _canceled = true;
    }

    // check if the queue is cancelled
    bool cancelled() {
        std::lock_guard<std::mutex> lock(_lock);
        return _canceled;
    }

    // peek at the top of the queue, remember to unlock if autoUnlock is false
    T& peek(bool autoUnlock = false) {
        lock();
        
        T& result = _queue.front();

        if(autoUnlock) {
            unlock();
        }

        return result;
    }

    // add an item to the queue
    void add(const T& item) {
        lock();

        _queue.push_back(item);

        unlock();
    }

    // add items back to the front of the queue
    template <class Iterator>
    void readd(Iterator begin, Iterator end) {
        std::lock_guard<std::mutex> lock(_lock);
        _queue.insert(_queue.begin(), begin, end);
    }

    // get the next result in the queue, if any
    bool next(T& result) {
        std::lock_guard<std::mutex> lock(_lock);

        if (_queue.empty()) {
            return false;
        }

        result = _queue.front();
        _queue.pop_front();

        return true;
    }

    template <class Checker>
    bool next(T& result, Checker& check) {
        std::lock_guard<std::mutex> lock(_lock);

        if (_queue.empty()) {
            return false;
        } 

        result = _queue.front();
        if (!check.Process(result)) {
            return false;
        }

        _queue.pop_front();
        return true;
    }
};

#endif // ! 