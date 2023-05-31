#include <server/loop.h>
#include <database/rdb.h>
#include <database/aof.h>

namespace rds
{
    MainLoop::MainLoop(const RedisConf &conf) : conf_(conf),
                                                server_(conf_.ip_.data(), conf_.port_),
                                                handler_(conf),
                                                file_manager_(conf.file_name_)
    {
        Log("Loading databases...");
        auto dbfile = file_manager_.LoadAndExport();
        file_manager_.Truncate();

        SetGlobalLoop(this);

        if (!conf.enable_aof_)
        {
            if (conf_.compress_)
            {
                EnCompress();
            }
            rdb_timer_.generator_ = [this]()
            { return this->DatabaseSave(); };
            rdb_timer_.expire_time_us_ = UsTime() + 1000'000;
            rdb_timer_.fm_ = &file_manager_;
            rdb_timer_.hdlr_ = &handler_;
            rdb_timer_.after_ = conf.frequence_.every_n_sec_ * 1000'000 / conf.frequence_.save_n_times_;

            handler_.Handle(std::make_unique<RdbTimer>(rdb_timer_));
            databases_ = RDBLoad(&dbfile);
            if (databases_.empty())
            {
                Log("Create a default database");
                databases_.push_back(std::make_unique<Db>());
            }
        }

        SetPassword(conf.password_);
        handler_.Run();

        if (conf.enable_aof_)
        {
            std::thread aof_load_thread(AOFLoad, conf.ip_, conf.port_, dbfile);
            aof_load_ = std::move(aof_load_thread);
        }
        Log("Inition works are done.");
    }

    void MainLoop::Run()
    {
        auto client_events = server_.Wait(-1);
        for (auto &client_event : client_events)
        {
            if (!client_event.first->GetDB())
            {
                Db *to_choose;
                {
                    std::lock_guard lg(db_mtx_);
                    to_choose = databases_.begin()->get();
                }
                client_event.first->SetDB(to_choose);
            }
            handler_.Handle(std::move(client_event));
        }
    }

    void MainLoop::AOFSyncWait()
    {
        std::unique_lock ul(aof_mtx_);
        Log("AOF wait for sync loading");
        aof_condv_.wait(ul, [&done = this->aof_load_flag_]()
                        { return done; });
    }
    void MainLoop::AOFSyncNotice()
    {
        std::lock_guard lg(aof_mtx_);
        Log("AOF finish sync loading and notice");
        aof_load_flag_ = true;
        aof_condv_.notify_all();
    }

    auto MainLoop::CheckAOFLoad() -> bool
    {
        if (!aof_load_.has_value())
        {
            return false;
        }
        AOFSyncNotice();
        aof_load_.value().join();
        if (databases_.empty())
        {
            Log("Create a default database");
            file_manager_.Write("AOF");
            databases_.push_back(std::make_unique<Db>());
        }

        aof_timer_.generator_ = [this]()
        { return this->DatabaseAppend(); };
        aof_timer_.fm_ = &file_manager_;
        aof_timer_.hdlr_ = &handler_;
        if (conf_.aof_mode_ == "every_sec")
        {
            aof_timer_.expire_time_us_ = UsTime() + 1000'000;
            aof_timer_.after_ = 1000'000;
            aof_timer_.every_sec_ = true;
        }
        else if (conf_.aof_mode_ == "always")
        {
            aof_timer_.expire_time_us_ = UsTime() + 1000;
            aof_timer_.after_ = 1000;
            aof_timer_.every_sec_ = true;
        }
        else
        {
            aof_timer_.expire_time_us_ = 0;
            aof_timer_.every_sec_ = false;
        }
        handler_.Handle(std::make_unique<AofTimer>(aof_timer_));
        return true;
    }

    auto MainLoop::DatabaseSave() const -> std::vector<std::string>
    {
        std::vector<std::string> ret;
        for (auto &db : databases_)
        {
            auto source = db->Save();
            ret.push_back(std::move(source));
        }
        return ret;
    }

    auto MainLoop::DatabaseAppend() const -> std::vector<std::string>
    {
        std::vector<std::string> ret;
        for (auto &db : databases_)
        {
            auto source = db->ExportAOF();
            ret.push_back(std::move(source));
        }
        return ret;
    }

    auto MainLoop::GetDB(int db_number) -> Db *
    {
        std::lock_guard lg(db_mtx_);
        for (auto &db : databases_)
        {
            if (db->Number() == db_number)
            {
                return db.get();
            }
        }
        return databases_.begin()->get();
    }

    auto MainLoop::CreateDB() -> int
    {
        std::lock_guard lg(db_mtx_);
        if (databases_.size() >= 16)
        {
            return -1;
        }
        auto db = std::make_unique<Db>();
        int n = db->Number();
        databases_.push_back(std::move(db));
        return n;
    }

    auto MainLoop::DropDB(int db_number) -> bool
    {
        std::lock_guard lg(db_mtx_);
        if (databases_.size() == 1)
        {
            return false;
        }
        for (auto it = databases_.begin(); it != databases_.end(); it++)
        {
            if ((*it)->Number() == db_number)
            {
                databases_.erase(it);
                return true;
            }
        }
        return false;
    }

    auto MainLoop::ShowDB() -> std::string
    {
        std::string ret;
        std::lock_guard lg(db_mtx_);
        for (auto &db : databases_)
        {
            ret.append(std::to_string(db->Number()));
            ret.append("|");
        }
        return ret;
    }

    auto MainLoop::SuGetDB(int db_number) -> Db *
    {
        std::lock_guard lg(db_mtx_);
        for (auto &db : databases_)
        {
            if (db->Number() == db_number)
            {
                return db.get();
            }
        }
        auto new_db = std::make_unique<Db>(db_number);
        auto ret = new_db.get();
        databases_.push_back(std::move(new_db));
        return ret;
    }

    static std::atomic<MainLoop *> g_loop_{nullptr};

    void SetGlobalLoop(MainLoop *g_loop)
    {
        g_loop_.store(g_loop);
    }

    auto GetGlobalLoop() -> MainLoop &
    {
        return *(g_loop_.load());
    }

    void MainLoop::EncounterTimer(std::unique_ptr<Timer> timer)
    {
        handler_.Handle(std::move(timer));
    }

    auto MainLoop::GetAOFTimer() -> AofTimer
    {
        std::lock_guard lg(aof_mtx_);
        return aof_timer_;
    }
    auto MainLoop::GetRDBTimer() -> RdbTimer
    {
        std::lock_guard lg(aof_mtx_);
        return rdb_timer_;
    }

} // namespace rds
