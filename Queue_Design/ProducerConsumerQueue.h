#ifndef  _PC_QUEUE_H
#define  _PC_QUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <type_traits>


template <typename T>
class ProducerConsumerQueue {
private:
    std::mutex _queueLock;
    std::queue<T> _queue;
    std::condition_variable _condition;
    std::atomic<bool> _shutdown;

public:
    
    ProducerConsumerQueue<T>(): _shutdown(false) {}

    void Push(const T& val) {
        std::lock_guard<std::mutex> lock(_queueLock);
        _queue.push(std::move(val));

        _condition.notify_one();
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(_queueLock);

        return _queue.empty();
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(_queueLock);

        return _queue.size();
    }

    bool Pop(T& val) {
        std::lock_guard<std::mutex> lock(_queueLock);

        if (_queue.empty() || _shutdown) {
            return false;
        }

        val = _queue.front();

        _queue.pop();

        return true;
    }

    void WaitAndPop(T& val) {
        std::unique_lock<std::mutex> lock(_queueLock);

        while (_queue.empty() && !_shutdown) {
            _condition.wait(lock);
        }

        if (_queue.empty() || _shutdown) {
            return;
        }

        val = _queue.front();

        _queue.pop();
    }

    void Cancel() {
        std::unique_lock<std::mutex> lock(_queueLock);

        while (!_queue.empty()) {

            T& val = _queue.front();

            DeleteQueuedObject(val);

            _queue.pop();
        }

        _shutdown = true;

        _condition.notify_all();
    }

private:
    template <typename E = T>
    typename std::enable_if<std::is_pointer_v<E>>::type 
    DeleteQueuedObject(E& obj) { 
        delete obj; 
    }
    template<typename E = T>
    typename std::enable_if<!std::is_pointer_v<E>>::type 
    DeleteQueuedObject(E const& /*packet*/) { }

};

#endif // ! _PC_QUEUE_H
