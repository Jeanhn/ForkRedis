#include <list>

/* rds::Server server;
rds::Handler handler;
rds::Rdb rdb;
std::list<std::unique_ptr<rds::Db>> databases;
rds::DiskManager dm;
rds::RdbTimer rdb_timer;

void run()
{
    auto dbfile = dm.LoadAndExport();
    dm.Truncate();

    databases = rds::Rdb::Load(&dbfile);
    rdb_timer.dm_ = &dm;
    rdb_timer.hdlr_ = &handler;
    rdb_timer.valid_ = true;
    rdb_timer.rdb_ = &rdb;
    rdb.SetSave(1, 1);
    rdb_timer.expire_time_us_ = rdb.Period() + rds::UsTime();

    handler.Push(std::make_unique<rds::RdbTimer>(rdb_timer));

    while (1)
    {
        auto readable = server.Wait(-1);
        for (auto &c : readable)
        {
            auto reqs = c->ExportMessages();
            if (reqs.empty())
            {
                continue;
            }
            server.EnableSend(c);
            for (auto &r : reqs)
            {
                auto cmd = rds::RequestToCommandExec(c, r);
                handler.Push(c, std::move(cmd));
                auto tmr = rds::ReqTimer(c, r);
                handler.Push(std::move(tmr));
            }
        }
        handler.Handle();
    }
} */

int main()
{
    int *p = new int;
    delete p;
}