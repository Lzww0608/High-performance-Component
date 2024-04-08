## Atomic-Lock —— 2024.3.27

+ 设计并测试并发编程时无锁、atomic、mutex时的不同情况。

+ 测试atomic的基本内存序：std::memory_order_relaxed, std::memory_order_release, std::memory_order_acquire, std::memory_acq_rel, std::memory_order_seq_cst。

+ 原子操作的保证问题, CPU读写数据，缓存一致性。

+ 互斥锁是如何实现的：内存标记线程ID、阻塞队列、硬件的原子操作、屏蔽中断、 (1.自旋 2.加入阻塞队列 3.休眠后尝试获取锁)


## Network Buffer Design ——— 2024.3.28
+ RingBuffer: gcc ringbuffer.c -c -fPIC    gcc -shared ringbuffer.o -o libringbuffer.so -I./-L

+ ChainBuffer: gcc chainbuffer.c -c -fPIC    gcc -shared chainbuffer.o -o libchainbuffer.so -I./-L


## Timers —— 2024.3.30
+ 模拟实现内核定时器
+ timers: epoll_wait()第四个参数timeout阻塞时间设置驱动定时器。 shell: g++ timers.cc -o timers -std=c++14


## Mempol —— 2024.3.31
+ mempool_fixed_block: 块大小固定，释放时间不固定。通过二级指针构造链表实现空余块之间的链接。
+ mempool_unfixed: 块大小不固定，每次分配当前块空间不足时会构建新的块。


## Deadlock Detection —— 2024.4.8
+ deadlock.c: 建立邻接图表来追踪和管理多线程中资源的使用情况并模拟线程之间的依赖关系，从而检测死锁的产生。shell: gcc deadlock.c -o deadlock -lpthread -ldl
