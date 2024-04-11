#include "msgqueue.h"

#include <thread>
#include <cstddef>
#include <iostream>


//侵入式
struct Count {
    Count(int _v): v(_v), next(nullptr) {}
    int v;
    Count *next;
};

int main() {
    // linkOff Count 偏移量为用于链接下一个节点的next指针
    msgqueue_t* queue = msgqueue_create(1024, sizeof(int));

    std::thread pt1([&]() {
        msgqueue_put(new Count(100), queue);
        msgqueue_put(new Count(200), queue);
        msgqueue_put(new Count(300), queue);
        msgqueue_put(new Count(400), queue);
    });

    std::thread pt2([&]() {
        msgqueue_put(new Count(500), queue);
        msgqueue_put(new Count(600), queue);
        msgqueue_put(new Count(700), queue);
        msgqueue_put(new Count(800), queue);
    });

    std::thread ck1([&]() {
        Count *cnt;
        while ((cnt = (Count *)msgqueue_get(queue)) != NULL) {
            std::cout << std::this_thread::get_id() << " pop: " << cnt->v << std::endl;
        }
    });

    std::thread ck2([&]() {
        Count *cnt;
        while ((cnt = (Count *)msgqueue_get(queue)) != NULL) {
            std::cout << std::this_thread::get_id() << " pop: " << cnt->v << std::endl;
        }
    });

    pt1.join();
    pt2.join();
    ck1.join();
    ck2.join();

    msgqueue_destroy(queue);

    return 0;
}