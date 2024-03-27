# Atomic-Lock —— 2024.3.27

+ 设计并测试并发编程时无锁、atomic、mutex时的不同情况。

+ 测试atomic的基本内存序：std::memory_order_relaxed, std::memory_order_release, std::memory_order_acquire, std::memory_acq_rel, std::memory_order_seq_cst。

+ 原子操作的保证问题, CPU读写数据，缓存一致性。

+ 互斥锁是如何实现的：内存标记线程ID、阻塞队列、硬件的原子操作、屏蔽中断、 (1.自旋 2.加入阻塞队列 3.休眠后尝试获取锁)
