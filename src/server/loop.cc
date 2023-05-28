#include <server/loop.h>
#include <database/rdb.h>
namespace rds
{
    MainLoop::MainLoop(const RedisConf &conf) : conf_(conf),
                                                server_(conf_.ip_.data(), conf_.port_),
                                                handler_(conf.cpu_num_),
                                                file_manager_(conf.file_name_)
    {
        Log("Loading databases...");
        auto dbfile = file_manager_.LoadAndExport();
        file_manager_.Truncate();

        if (conf.enable_aof_)
        {
            // databases_ = Aof::Load(&dbfile);
        }
        else
        {
            if (conf_.compress_)
            {
                EnCompress();
            }
            databases_ = RDBLoad(&dbfile);
            RdbTimer timer;
            timer.generator_ = [this]()
            { return this->DatabaseFork(); };
            timer.expire_time_us_ = UsTime() + 1000'000;
            timer.fm_ = &file_manager_;
            timer.hdlr_ = &handler_;
            timer.after_ = conf.frequence_.every_n_sec_ * 1000'000 / conf.frequence_.save_n_times_;

            handler_.Handle(std::make_unique<RdbTimer>(timer));
        }

        if (databases_.empty())
        {
            Log("Create a default database");
            databases_.push_back(std::make_unique<Db>());
        }
        handler_.Run();
#ifndef NDEBUG
        handler_.Run();
#endif
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

    auto MainLoop::DatabaseFork() const -> std::vector<std::string>
    {
        std::vector<std::string> ret;
        for (auto &db : databases_)
        {
            auto source = db->Save();
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

    static std::atomic<MainLoop *> g_loop_;

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

} // namespace rds
