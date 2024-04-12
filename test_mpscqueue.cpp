#include "MPSCQueue.h"

#include <thread>
#include <iostream>
#include <memory>

struct Count {
    Count(int _v): v(_v) {}
    int v;
};


int main() {
    MPSCQueue<Count> queue;
    
    std::thread pt1([&]() {
        queue.Enqueue(new Count(100));
        queue.Enqueue(new Count(200));
        queue.Enqueue(new Count(300));
        queue.Enqueue(new Count(400));
    });

    std::thread pt2([&]() {
        queue.Enqueue(new Count(500));
        queue.Enqueue(new Count(600));
        queue.Enqueue(new Count(700));
        queue.Enqueue(new Count(800));
    });

    std::thread ck([&]() {
        Count* ptr;
        while (queue.Dequeue(ptr)) {
            std::cout << std::this_thread::get_id() << " pop: " << ptr->v << std::endl;
            delete(t);
        }
    });

    pt1.join();
    pt2.join();
    ck.join();

    return 0;
}