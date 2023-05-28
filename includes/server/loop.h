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
        std::mutex db_mtx_;
        std::list<std::unique_ptr<Db>> databases_;

        Server server_;
        Handler handler_;
        FileManager file_manager_;

        std::unique_ptr<std::thread> save_thread_;

    public:
        void Run();
        auto DatabaseFork() const -> std::vector<std::string>;

        auto GetDB(int db_number) -> Db *;
        auto CreateDB() -> int;
        auto DropDB(int db_number) -> bool;
        auto ShowDB() -> std::string;

        void EncounterTimer(std::unique_ptr<Timer> timer);

        MainLoop(const RedisConf &conf);
        ~MainLoop() = default;
    };
    void SetGlobalLoop(MainLoop *g_loop);
    auto GetGlobalLoop() -> MainLoop &;
} // namespace rds

#endif