#ifndef __HANDLER_H__
#define __HANDLER_H__

#include <util.h>
#include <server/command.h>
#include <server/timer.h>
#include <server/server.h>

namespace rds
{
    class Handler
    {
    private:
        const RedisConf conf_;

        std::atomic_bool running_{false};

        CommandQue cmd_que_;

        TimerQue tmr_que_;

        static void ExecCommand(Handler *hdlr);
        static void ExecTimer(Handler *hdlr);
        static void ioClient(Handler *hdlr);

        std::mutex cli_mtx_;
        std::condition_variable condv_;
        std::queue<std::pair<std::weak_ptr<ClientInfo>, int>> cli_que_;

        std::list<std::thread> workers_;

    public:
        void Run();
        void Handle(std::pair<std::weak_ptr<ClientInfo>, int> cli_evt);
        void Handle(std::unique_ptr<Timer> timer);
        void Stop();
        Handler(int io_thread_num);
        Handler(const RedisConf &conf);
        CLASS_DECLARE_uncopyable(Handler);
    };
} // namespace rds

#endif