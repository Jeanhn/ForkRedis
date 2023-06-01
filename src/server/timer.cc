#include <server/timer.h>
#include <server/handler.h>
#include <server/server.h>

namespace rds
{
    void DbExpireTimer::Exec()
    {
        database_->Del({obj_name_});
    }

    void RdbTimer::Exec()
    {
        auto src = generator_();
        RDBSave(std::move(src), fm_);
        expire_time_us_ = UsTime() + after_;
        hdlr_->Handle(std::make_unique<RdbTimer>(*this));
    }

    void AofTimer::Exec()
    {
        auto src = generator_();
        AOFSave(std::move(src), fm_);
        expire_time_us_ = UsTime() + after_;
        if (every_sec_)
        {
            hdlr_->Handle(std::make_unique<AofTimer>(*this));
        }
    }

    void DbRewriteTimer::Exec()
    {
        fm_->Truncate();
        fm_->Write(std::move(cache_));
    }

    void TimerQue::Push(std::unique_ptr<Timer> timer)
    {
        std::lock_guard<std::mutex> lg(mtx_);
        que_.push(std::move(timer));
        condv_.notify_one();
    }

    auto TimerQue::BlockPop() -> std::unique_ptr<Timer>
    {
        std::unique_lock<std::mutex> ul(mtx_);
        while (true)
        {
            if (!que_.empty())
            {
                auto now_us = UsTime();
                if (que_.top()->expire_time_us_ > now_us)
                {
                    std::size_t sleep_period = que_.top()->expire_time_us_ - now_us;
                    condv_.wait_until(ul,
                                      std::chrono::system_clock::now() +
                                          std::chrono::microseconds(sleep_period));
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                condv_.wait(ul, [&que = que_]()
                            { return !que.empty(); });
            }
        }

        auto ret = std::move(*(const_cast<std::unique_ptr<Timer> *>(&que_.top())));
        que_.pop();
        return ret;
    }
} // namespace rds
