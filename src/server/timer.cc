#include <server/timer.h>
#include <server/server.h>

namespace rds
{
    void DbExpireTimer::Exec()
    {
        database_->Del({obj_name_});
    }

    void RdbTimer::Exec()
    {
        auto newtmr = std::make_unique<RdbTimer>(*this);
        newtmr->expire_time_us_ = rdb_->Period() + UsTime();
        hdlr_->Push(std::move(newtmr));
        auto str = rdb_->Save();
        fm_->Write(str);
    }

} // namespace rds
