#ifndef __LOOP_H__
#define __LOOP_H__
#include <server/server.h>
#include <server/timer.h>
#include <server/command.h>
#include <database/db.h>
#include <database/disk.h>
#include <database/rdb.h>
#include <thread>
namespace rds
{
    class MainLoop
    {
    public:
        const RedisConf &conf_;

    private:
        std::list<std::unique_ptr<Db>> databases_;
        FileManager file_manager_;
        Server server_;
        Handler handler_;

        std::unique_ptr<std::thread> save_thread_;

    public:
        void Run();
        auto RdbSave() const -> std::string;
        MainLoop(const RedisConf &conf);
        ~MainLoop() = default;
    };
} // namespace rds

#endif