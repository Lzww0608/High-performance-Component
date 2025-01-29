#include <sys/epoll.h>
#include <iostream>
#include <functional>
#include <set>
#include <memory>
#include <chrono>

//暴露给用户的类
struct TimerNodeBase {
    time_t expire;
    int64_t id;
};

//拓展至Timer类中的protected
struct TimerNode: public TimerNodeBase {
    using Callback = std::function<void(const TimerNode& node)>;
    Callback func;
    TimerNode(int64_t id, time_t expire, Callback func): func(func) {
        this->id = id;
        this->expire = expire;
    }
};
//基类的引用同基类的指针，具有多态性
//TimerNode TimerNode
//TimerNodeBase TimerNodeBase
//TimerNode TimerNodeBase
//TimerNodeBase TimerNode
bool operator < (const TimerNodeBase &lhd, const TimerNodeBase &rhd) {
    if (lhd.expire == rhd.expire) {
        return lhd.id < rhd.id;
    }
    return lhd.expire < rhd.expire;
}


class Timer {
public:
    // 获取当前时间戳(以毫秒为单位)
    // 使用std::chrono::steady_clock保证时钟单调递增
    // 返回自系统启动以来的毫秒数
    static time_t GetTick() {
        // 获取当前时间点并转换为毫秒精度
        auto sc = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
        // 将时间点转换为自epoch以来的时间间隔(毫秒)
        auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(sc.time_since_epoch());
        // 返回毫秒计数
        return tmp.count();
    }
    //std::bind绑定对象
    //仿函数
    //lambda表达式
    // expire 相同 O(logn) -> O(1) 总是向红黑树右侧添加节点
    TimerNodeBase AddTimer(time_t msec, TimerNode::Callback&& func) {
        time_t expire = GetTick() + msec;
        if (timeouts.empty() || expire <= timeouts.crbegin()->expire) {
            auto pairs = timeouts.emplace(GetID(), expire, std::move(func));
            //pair<*TimerNode, bool>
            return static_cast<TimerNodeBase>(*pairs.first);
        }
        auto ele = timeouts.emplace_hint(timeouts.end(), GetID(), expire, std::move(func));
        return static_cast<TimerNodeBase>(*ele);
    }

    bool DelTimer(TimerNodeBase &node) {
        auto it = timeouts.find(node);
        if (it != timeouts.end()) {
            timeouts.erase(it);
            return true;
        }
        return false;
    }

    void HandleTimer(time_t now) {
        auto it = timeouts.begin();
        while (it != timeouts.end() && it->expire <= now) {
            it->func(*it);
            it = timeouts.erase(it);
        }
    }

    time_t TimeToSleep() {
        auto it = timeouts.begin();
        if (it == timeouts.end()) {
            return -1;
        }
        time_t t = it->expire - GetTick();
        return t > 0 ? t : 0;
    }


private:
    static int64_t tid;
    static int64_t GetID() {
        return tid++;
    }
    std::set<TimerNode, std::less<>> timeouts;
};

int64_t Timer::tid = 0;

int main() {
    int epfd = epoll_create(1);

    std::unique_ptr<Timer> timer = std::make_unique<Timer>();

    int i = 0;
    timer->AddTimer(1000, [&](const TimerNode &node) {
        std::cout << Timer::GetTick() << " node id:" << node.id << " revoked times:" << ++i << std::endl;
    });

    timer->AddTimer(1000, [&](const TimerNode &node) {
        std::cout << Timer::GetTick() << " node id:" << node.id << " revoked times:" << ++i << std::endl;
    });

    timer->AddTimer(3000, [&](const TimerNode &node) {
        std::cout << Timer::GetTick() << " node id:" << node.id << " revoked times:" << ++i << std::endl;
    });

    auto node = timer->AddTimer(2100, [&](const TimerNode &node) {
        std::cout << Timer::GetTick() << " node id:" << node.id << " revoked times:" << ++i << std::endl;
    });

    timer->DelTimer(node);

    std::cout << "now time: " << Timer::GetTick() << std::endl;
    epoll_event ev[64] = {0};

    while (true) {
        int n = epoll_wait(epfd, ev, 64, timer->TimeToSleep());
        time_t now = Timer::GetTick();
        for (int i = 0; i < n; i++) {

        }
        /*处理定时事件*/
        timer->HandleTimer(now);
    }
    return 0;
}