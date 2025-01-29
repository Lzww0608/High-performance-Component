#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>   // for timespec itimerspec
#include <unistd.h> // for close

#include <functional>
#include <chrono>
#include <set>
#include <memory>
#include <iostream>

using namespace std;

struct TimerNodeBas {
    time_t expire;
    uint64_t id;
};

struct TimerNode: public TimerNodeBase {
    using Callback = std::function<void(const TimerNode& node)>;
    Callback func;
    TimerNode(int64_t id, time_t expire, Callback func): func(func) {
        this->id = id;
        this->expire = expire;
    }
};

bool operator < (const TimerNodeBase& lhd, const TimerNodeBase& rhd) {
    if (lhd.expire < rhd.expire) {
        return true;
    } else if (lhd.expire > rhd.expire) {
        return false;
    } else return lhd.id < rhd.id;
}

class Timer {
public:
    static inline time_t GetTick() {
        return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
    }

    TimerNodeBase AddTimer(int msec, TimerNode::Callback func) {
        time_t expire = GetTick() + msec;
        if (timeouts.empty() || expire <= timeouts.cbegin()->expire) {
            auto pairs = timeouts.emplace(GenID(), expire, std::move(func));
            return static_cast<TimerNodeBase>(*pairs.first);
        }
        auto ele = timeouts.emplace_hint(timeouts.crbegin().base(), GenID(), expire, std::move(func));
        return static_cast<TimerNodeBase>(*ele)
    }

    void DelTimer(TimerNodeBase &node) {
        auot it =  timeouts.find(node);
        if (it != timeouts.end()) {
            timeouts.erase(it);
        }
    }

private:
    static inline unint64_t GenID() {
        return gid++;
    }
    static uint64_t gid;
    set<TimerNode, std::less<>> timeouts;
};