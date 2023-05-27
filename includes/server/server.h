#ifndef __SERVER_H__
#define __SERVER_H__

#include <util.h>
#include <server/command.h>
#include <server/timer.h>
#include <queue>
#include <sys/epoll.h>
#include <database/db.h>
#include <list>

namespace rds
{

    class Server;
    class Handler;

    class ClientInfo
    {
        friend Server;
        friend Handler;
#ifdef NDEBUG
    private:
#else
    public:
#endif
        int fd_;
        Db *database_;

        std::deque<char> recv_buffer_;
        std::vector<json11::Json::array> recv_messages_;

        std::deque<char> send_buffer_;
        std::vector<json11::Json::array> send_messages_;

        auto Read() -> int;
        auto Send() -> int;

    public:
        void ShiftDB(Db *database);
        auto GetDB() -> Db *;
        void Append(json11::Json::array to_send_message);
        auto IsSendOut() -> bool;
        auto ExportMessages() -> std::vector<json11::Json::array>;
        ClientInfo(int fd);
        ~ClientInfo();
    };

    class Server
    {
    private:
        std::unordered_map<int, std::unique_ptr<ClientInfo>> client_map_;
        std::vector<epoll_event> epoll_revents_;
        int listen_fd_;
        int epfd_;

    public:
        auto Wait(int timeout) -> std::vector<ClientInfo *>;
        void EnableSend(ClientInfo *client);
        Server(const char *ip = "127.0.0.1", short port = 8080);
        ~Server();
    };

    class Handler
    {
    private:
        std::atomic_bool running_{false};

        CommandQue cmd_que_;

        TimerQue tmr_que_;

        void ExecCommand();
        void ExecTimer();

        std::list<std::thread> workers_;

    public:
        void Push(ClientInfo *client, std::unique_ptr<CommandBase> cmd);
        void Push(std::unique_ptr<Timer> timer);
        void Run();
        void Stop();

        CLASS_DEFAULT_DECLARE(Handler);
    };

} // namespace rds

#endif