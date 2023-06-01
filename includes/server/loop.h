#ifndef __LOOP_H__
#define __LOOP_H__
#include <server/server.h>
#include <server/timer.h>
#include <server/command.h>
#include <server/handler.h>
#include <database/db.h>
#include <database/disk.h>
#include <database/rdb.h>
#include <thread>
namespace rds
{
    class MainLoop
    {
    public:
        const RedisConf conf_;

    private:
        mutable std::shared_mutex db_mtx_;
        std::list<std::unique_ptr<Db>> databases_;

        Server server_;
        Handler handler_;
        FileManager file_manager_;

        std::unique_ptr<std::thread> save_thread_;

        RdbTimer rdb_timer_;
        AofTimer aof_timer_;

        std::optional<std::thread> aof_load_;
        bool aof_load_flag_{false};
        std::mutex aof_mtx_;
        std::condition_variable aof_condv_;

    public:
        void Run();
        auto DatabaseSave() const -> std::vector<std::string>;
        auto DatabaseAppend() const -> std::vector<std::string>;
        auto DatabaseForkAll() const -> std::string;

        auto GetDB(int db_number) const -> Db *;
        auto CreateDB() -> int;
        auto DropDB(int db_number) -> bool;
        auto ShowDB() const -> std::string;

        auto SuGetDB(int db_number) -> Db *;

        void EncounterTimer(std::unique_ptr<Timer> timer);

        auto CheckAOFLoad() -> bool;
        void AOFSyncWait();
        void AOFSyncNotice();
        auto GetAOFTimer() -> AofTimer;
        auto GetRDBTimer() -> RdbTimer;

        MainLoop(const RedisConf &conf);
        ~MainLoop() = default;
    };
    void SetGlobalLoop(MainLoop *g_loop);
    auto GetGlobalLoop() -> MainLoop &;
} // namespace rds

#endif