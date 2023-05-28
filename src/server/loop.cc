#include <server/loop.h>

namespace rds
{
    MainLoop::MainLoop(const RedisConf &conf) : conf_(conf),
                                                server_(conf_.ip_.data(), conf_.port_)
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
            // databases_ = Rdb::Load(&dbfile);
            // Rdb rdb([this]()
            //         { return this->RdbSave(); });
        }

        if (databases_.empty())
        {
            Log("Create a default database");
            databases_.push_back(std::make_unique<Db>());
        }
        handler_.Run();
    }

    void MainLoop::Run()
    {
        auto client_events = server_.Wait(-1);
        for (auto &client_event : client_events)
        {
            if (!client_event.first->GetDB())
            {
                client_event.first->SetDB(databases_.begin()->get());
            }
            handler_.Handle(std::move(client_event));
        }
    }
} // namespace rds
