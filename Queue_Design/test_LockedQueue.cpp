#include "LockedQueue.h"

#include <thread>
#include <iostream>

class Check1 {
public:
    bool Process(int val) {
        return (val & 1) == 0;
    }

};

class Check2 {
public:
    bool Process(int val) {
        return (val & 1) == 1;
    }

};



int main() {

    LockedQueue<int> queue;

    std::thread pt1([&]() {
        queue.add(10);
        queue.add(2);
        queue.add(3);
        queue.add(4);
    });
    
    std::thread pt2([&]() {
        queue.add(5);
        queue.add(6);
        queue.add(7);
        queue.add(8);
    });

    std::thread ck1([&]() {
        int v;
        Check1 check;
        while (queue.next(v, check)) {
            std::cout << std::this_thread::get_id() << " pop: " << v << std::endl;
        }
    });

    std::thread ck2([&]() {
        int v;
        Check2 check;
        while (queue.next(v, check)) {
            std::cout << std::this_thread::get_id() << " pop: " << v << std::endl;
        }
    });

    pt1.join();
    pt2.join();
    ck1.join();
    ck2.join();
    

    return 0;

}