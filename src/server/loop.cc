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
            databases_ = Aof::Load(&dbfile);
        }
        else
        {
            databases_ = Rdb::Load(&dbfile);
            Rdb rdb([this]()
                    { return this->RdbSave(); });
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
        auto clients = server_.Wait(-1);
        for (auto &client : clients)
        {
            auto reqs = client->ExportMessages();
            if (reqs.empty())
            {
                continue;
            }
            server_.EnableSend(client);
            for (auto &req : reqs)
            {
                auto cmd = RequestToCommandExec(client, req);
                if (cmd)
                {
                    if (!client->database_)
                    {
                        client->database_ = databases_.begin()->get();
                    }
                    handler_.Push(client, std::move(cmd));
                }
            }
        }
    }
} // namespace rds
