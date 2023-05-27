#include <server/timer.h>
#include "util4test.h"

TEST(Timer, TimerQue)
{
    using namespace rds;
    // std::vector<std::chrono::steady_clock::time_point> time_points;
    // for (std::size_t i = 1; i < 10; i++)
    // {
    //     auto p = std::chrono::steady_clock::now() + std::chrono::milliseconds(i * 1000);
    //     time_points.push_back(p);
    // }
    TimerQue que;
    for (std::size_t i = 1; i < 10; i++)
    {
        auto tmr = std::make_unique<Timer>();
        tmr->expire_time_us_ = UsTime() + i * 1000'000;
        que.Push(std::move(tmr));
    }
    std::atomic_bool reach(false);
    std::thread t([&reach](TimerQue *q) mutable
                  { auto start=UsTime();
        int loop = 10;
        while (loop)
        {
            loop--;
            reach = true;
            auto rmr = q->BlockPop();
            Log("Get Timer:", double(UsTime() - start) / 1000'000);
        } },
                  &que);
    while (!reach)
        ;
    auto tmr = std::make_unique<Timer>();
    tmr->expire_time_us_ = UsTime() + 500'000;
    que.Push(std::move(tmr));
    t.join();
}