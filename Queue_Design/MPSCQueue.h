
#ifndef _MPSCQueue_H
#define _MPSCQueue_H

#include <atomic>
#include <utility>
#include <type_traits>
#include <memory>

//wait-free queue
//nonintrusive
template <typename T>
class MPSCQueueNonIntrusive {
public:
    MPSCQueueNonIntrusive(): _head(new Node()), _tail(_head.load(std::memory_order_relaxed)) {
        Node* front = _head.load(std::memory_order_relaxed);
        front->Next.store(nullptr, std::memory_order_relaxed);
    }

    ~MPSCQueueNonIntrusive() {
        T* output;
        while (Dequeue(output)) {
            delete output;
        }

        Node* front = _head.load(std::memory_order_relaxed);
        delete front;
    }

    void Enqueue(T* input) {
        Node* node = new Node(input);
        Node* prevHead = _head.exchange(node, std::memory_order_acq_rel);
        prevHead->Next.store(node, std::memory_order_release);
    }

    //std::memory_order_release之前的所有写入操作都不会被编译器或处理器重排到该原子操作之后
    // std::memory_order_acquire 的主要作用是防止后续的读或写操作被重排到原子操作之前
    bool Dequeue(T* &result) {
        Node* tail = _tail.load(std::memory_order_relaxed);
        Node* next = tail->Next.load(std::memory_order_acquire);
        if (next == nullptr) {
            return false;
        }

        result = next->Data;
        _tail.store(next, std::memory_order_release);
        delete tail;
        return true;
    }


private:
    struct Node {
        Node() = default;
        explicit Node(T* data): Data(data) {
            Next.store(nullptr, std::memory_order_relaxed);
        }

        T* Data;
        std::atomic<Node*> Next;
    };

    std::atomic<Node*> _head;
    std::atomic<Node*> _tail;

    MPSCQueueNonIntrusive(MPSCQueueNonIntrusive const&) = delete;
    MPSCQueueNonIntrusive& operator=(MPSCQueueNonIntrusive const&) = delete;
    /*
    MPSCQueueNonIntrusive(MPSCQueueNonIntrusive const&&) = delete;
    MPSCQueueNonIntrusive& operator=(MPSCQueueNonIntrusive const&&) = delete;
    */
};

//
template <typename T, std::atomic<T*> T::*IntrusiveLink>
class MPSCQueueIntrusive {
public:
    MPSCQueueIntrusive(): _dummyPtr(reinterpret_cast<T*>(std::addressof(_dummy))), _head(_dummyPtr), _tail(_dummyPtr) {
        // _dummy is constructed from aligned_storage and is intentionally left uninitialized (it might not be default constructible)
        // so we init only its IntrusiveLink here
        std::atomic<T*>* dummyNext = new (&(_dummyPtr->*IntrusiveLink)) std::atomic<T*>();
        dummyNext->store(nullptr, std::memory_order_relaxed);
    }

    ~MPSCQueueIntrusive() {
        T* output;
        while (Dequeue(output)) {
            delete output;
        }
    }

    void Enqueue(T* input) {
        (input->*IntrusiveLink).store(nullptr, std::memory_order_release);
        T* preHead = _head.exchange(input, std::memory_order_acq_rel);
        (preHead->*IntrusiveLink).store(input, std::memory_order_release);
    }

    bool Dequeue(T* &result) {
        T* tail = _tail.load(std::memory_order_relaxed);
        T* next = (tail->*IntrusiveLink).load(std::memory_order_acquire);
        
        if (tail == _dummyPtr) {
            if (next == nullptr) {
                return false;
            }

            _tail.store(next, std::memory_order_release);
            tail = next;
            next = (next->*IntrusiveLink).load(std::memory_order_acquire);
        }

        if (next != nullptr) {
            _tail.store(next, std::memory_order_release);
            result = tail;
            return true;
        }

        T* head = _head.load(std::memory_order_acquire);
        if (tail != head) {
            return false;
        }

        Enqueue(_dummyPtr);
        next = (tail->*IntrusiveLink).load(std::memory_order_acquire);
        if (next != nullptr) {
            _tail.store(next, std::memory_order_release);
            result = tail;
            return true;
        }

        return false;
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> _dummy;
    T* _dummyPtr;
    std::atomic<T*> _head;
    std::atomic<T*> _tail;

    MPSCQueueIntrusive(MPSCQueueIntrusive const&) = delete;
    MPSCQueueIntrusive& operator=(MPSCQueueIntrusive const&) = delete;
};


template <typename T, std::atomic<T*> T:: *IntrusiveLink = nullptr>

using MPSCQueue = std::conditional_t<IntrusiveLink != nullptr, MPSCQueueIntrusive<T, IntrusiveLink>, MPSCQueueNonIntrusive<T>>;

#endif // !_MPSCQueue_H