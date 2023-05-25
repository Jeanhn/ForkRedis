#include <rds.h>

rds::Server server("127.0.0.1", 8080);
rds::Handler handler;
rds::Rdb rdb;
std::list<std::unique_ptr<rds::Db>> databases;
rds::FileManager fm;
rds::RdbTimer rdb_timer;

int main()
{
    std::cout << "loading rdb file ..." << std::endl;
    auto dbfile = fm.LoadAndExport();
    fm.Truncate();

    databases = rds::Rdb::Load(&dbfile);

    rdb.SetSaveFrequency(1, 1);
    rdb.Manage(&databases);

    rdb_timer.fm_ = &fm;
    rdb_timer.hdlr_ = &handler;
    rdb_timer.valid_ = true;
    rdb_timer.rdb_ = &rdb;
    rdb_timer.expire_time_us_ = rdb.Period() + rds::UsTime();

    handler.Push(std::make_unique<rds::RdbTimer>(rdb_timer));

    std::cout << "waiting for client" << std::endl;

    databases.push_back(std::make_unique<rds::Db>());

    server.db_source_ = &databases;

    while (1)
    {
        rds::Log("Waiting for connections...");
        auto readable = server.Wait(-1);
        rds::Log("Active clients come:", readable.size());
        for (auto &c : readable)
        {
            auto reqs = c->ExportMessages();
            rds::Log("Client req size:", reqs.size());
            for (auto &j : reqs)
            {
                json11::Json js = j;
                rds::Log("req:", js.dump());
            }
            if (reqs.empty())
            {
                continue;
            }
            server.EnableSend(c);
            for (auto &r : reqs)
            {
                auto cmd = rds::RequestToCommandExec(c, r);
                if (cmd)
                {
                    handler.Push(c, std::move(cmd));
                }
                auto tmr = rds::ReqTimer(c, r);
                if (tmr)
                {
                    handler.Push(std::move(tmr));
                }
            }
        }
        handler.Handle();
    }

    fm.Truncate();
}