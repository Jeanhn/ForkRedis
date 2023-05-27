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
    class MainLoop;

    class ClientInfo
    {
#ifdef NDEBUG
    private:
#else
    public:
#endif
        const int fd_;
        Server *server_;
        Db *database_;

        std::deque<char> recv_buffer_;
        std::vector<json11::Json::array> recv_messages_;

        std::deque<char> send_buffer_;
        std::vector<json11::Json::array> send_messages_;

        std::shared_mutex latch_;

    public:
        auto GetFD() -> int;
        auto Read() -> int;
        auto Send() -> int;
        void SetDB(Db *database);
        auto GetDB() -> Db *;
        void Append(json11::Json::array to_send_message);
        auto IsSendOut() -> bool;
        auto ExportMessages() -> std::vector<json11::Json::array>;
        void EnableSend();
        ClientInfo(Server *server = nullptr, int fd = -1);
        ~ClientInfo();
    };

    class Server
    {
    private:
        std::unordered_map<int, std::shared_ptr<ClientInfo>> client_map_;
        std::vector<epoll_event> epoll_revents_;
        int listen_fd_;
        int epfd_;

    public:
        auto Wait(int timeout) -> std::vector<std::shared_ptr<ClientInfo>>;
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

        static void ExecCommand(Handler *hdlr);
        static void ExecTimer(Handler *hdlr);
        static void HandleClient(Handler *hdlr);

        std::mutex cli_mtx_;
        std::condition_variable condv_;
        std::queue<std::weak_ptr<ClientInfo>> cli_que_;

        std::list<std::thread> workers_;

    public:
        void Run();
        void Handle(std::weak_ptr<ClientInfo> client);
        void Stop();

        CLASS_DEFAULT_DECLARE(Handler);
    };

} // namespace rds

#endif