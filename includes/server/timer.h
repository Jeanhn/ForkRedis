#ifndef __TIMER_H__
#define __TIMER_H__

#include <util.h>
#include <database/db.h>
#include <json11.hpp>
#include <database/rdb.h>
#include <database/aof.h>
#include <database/disk.h>

namespace rds
{
    struct Timer
    {
        bool valid_;
        std::size_t expire_time_us_;
        virtual void Exec() = 0;
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

} // namespace rds

#endif