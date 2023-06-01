#ifndef __TIMER_H__
#define __TIMER_H__

#include <util.h>
#include <database/db.h>
#include <json11.hpp>
#include <database/rdb.h>
#include <database/aof.h>
#include <database/disk.h>
#include <condition_variable>
#include <queue>

namespace rds
{
    struct Timer
    {
        std::size_t expire_time_us_;
        virtual void Exec(){};
        CLASS_DEFAULT_DECLARE(Timer);
    };

    struct DbExpireTimer : Timer
    {
        Db *database_;
        std::string obj_name_;
        void Exec() override;
        CLASS_DEFAULT_DECLARE(DbExpireTimer);
    };

    class Handler;

    struct DbRewriteTimer : Timer
    {
        FileManager *fm_;
        std::string cache_;
        void Exec() override;
    };

    struct RdbTimer : Timer
    {
        std::function<std::vector<std::string>()> generator_;
        FileManager *fm_;
        Handler *hdlr_;
        std::size_t after_;
        void Exec() override;
        CLASS_DEFAULT_DECLARE(RdbTimer);
    };

    struct AofTimer : Timer
    {
        std::function<std::vector<std::string>()> generator_;
        std::string ip_;
        short port_;
        FileManager *fm_;

        std::size_t after_;
        Handler *hdlr_;
        bool every_sec_{true};
        void Exec();
    };

    auto NewAOFTimer() -> AofTimer;
    auto NewRDBTimer() -> RdbTimer;

    class ClientInfo;

    inline auto ReqTimer(ClientInfo *, json11::Json::array) -> std::unique_ptr<Timer>
    {
        return nullptr;
    }

    inline auto TimerLess(const std::unique_ptr<Timer> &a, const std::unique_ptr<Timer> &b) -> bool
    {
        return a->expire_time_us_ < b->expire_time_us_;
    }

    inline auto TimerGreater(const std::unique_ptr<Timer> &a, const std::unique_ptr<Timer> &b) -> bool
    {
        return a->expire_time_us_ > b->expire_time_us_;
    }

    class TimerQue
    {
    private:
        /* data */
        std::mutex mtx_;
        std::condition_variable condv_;
        std::priority_queue<std::unique_ptr<Timer>, std::vector<std::unique_ptr<Timer>>,
                            decltype(&TimerGreater)>
            que_{TimerGreater};

    public:
        void Push(std::unique_ptr<Timer> timer);
        auto BlockPop() -> std::unique_ptr<Timer>;
        TimerQue(/* args */) = default;
        ~TimerQue() = default;
        CLASS_DECLARE_uncopyable(TimerQue);
    };

} // namespace rds

#endif