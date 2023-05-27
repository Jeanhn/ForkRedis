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
        bool valid_;
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

    struct RdbTimer : Timer
    {
        Handler *hdlr_;
        Rdb *rdb_;
        FileManager *fm_;
        void Exec() override;
        CLASS_DEFAULT_DECLARE(RdbTimer);
    };

    struct AofTimer : Timer
    {
        Handler *hdlr_;
        Aof *aof_;
        FileManager *dm_;
        void Exec();
    };

    class ClientInfo;

    inline auto ReqTimer(ClientInfo *, json11::Json::array) -> std::unique_ptr<Timer>
    {
        return nullptr;
    }

    auto NewRdbTimer(Handler *hdlr_, Rdb *rdb_) -> std::unique_ptr<Timer>;

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