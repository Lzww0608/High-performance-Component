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


## Memmory_Leak —— 2024.4.8
+ memleak.c: 单文件检测内存泄漏方式(1)自定义`malloc`函数，通过打印信息 (2)`malloc`时创建文件，`free`时销毁文件，通过文件是否存在判断  并设置标志位作为启动标志
+ memleak_hook.c: 多文件检测方式，自定义hook，打印错误信息文件(缺点：文件的频繁创建与销毁)
+ shell: gcc memleak_hook.c -o memleak_hook -ldl -g  (显示泄漏代码行号): addr2line -f -e ./memleak -a `address`


## Distributed_Lock —— 2024.4.9
+ 分布式锁：互斥型、锁超时、可用性、容错性。网络通信方式进行加锁、解锁。
+ MySQL：利用MySQL唯一键的唯一约束性来实现互斥性。
+ Redis实现非公平锁：Forked From jacketzhong C++实现基于Hiredis的分布式锁


## Queue_Design —— 2024.4.10
+ LockedQueue: 设计有锁队列并测试(无wait功能)。
+ ProduecerConsumerQueue: 设计有锁生产者-消费者队列并测试(有wait功能)。shell(需安装Google Test): g++ -std=c++17 -pthread test_ProducerConsumerQueue.cpp -lgtest -lgtest_main -o test
+ msgqueue: 设计有锁消息队列并测试。 shell: g++ -std=c++ -pthread msgqueue.c test_msgqueue.cpp -o test
