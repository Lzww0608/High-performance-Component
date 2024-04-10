#include "ProducerConsumerQueue.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

class ProducerConsumerQueueTest : public ::testing::Test {
protected:
    ProducerConsumerQueue<int> queue;

    void SetUp() override {
        // 初始化代码，每个测试用例开始前执行
    }

    void TearDown() override {
        // 清理代码，每个测试用例结束后执行
    }
};

TEST_F(ProducerConsumerQueueTest, SingleThreadedEnqueueDequeue) {
    queue.Push(1);
    queue.Push(2);
    
    int result;
    ASSERT_TRUE(queue.Pop(result));
    EXPECT_EQ(result, 1);
    
    ASSERT_TRUE(queue.Pop(result));
    EXPECT_EQ(result, 2);
    
    ASSERT_FALSE(queue.Pop(result));  // 应该为空
}

TEST_F(ProducerConsumerQueueTest, MultiThreadedEnqueueDequeue) {
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::vector<int> consumedItems;
    std::mutex mtx; //包括vector多线程安全

    for (int i = 0; i < 5; ++i) {
        producers.push_back(std::thread([&]() {
            for (int j = 0; j < 100; ++j) {
                queue.Push(j);
            }
        }));
    }

    for (int i = 0; i < 5; ++i) {
        consumers.push_back(std::thread([&]() {
            for (int j = 0; j < 100; ++j) {
                int item;
                if (queue.Pop(item)) {
                    std::lock_guard<std::mutex> lock(mtx);
                    consumedItems.push_back(item);
                }
            }
        }));
    }

    for (auto& producer : producers) {
        producer.join();
    }

    for (auto& consumer : consumers) {
        consumer.join();
    }

    EXPECT_EQ(consumedItems.size(), 500); // 检查是否正确消费了所有项目
}

TEST_F(ProducerConsumerQueueTest, DequeueFromEmptyQueue) {
    int item;
    ASSERT_FALSE(queue.Pop(item)); // 确保从空队列取数据返回 false
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
