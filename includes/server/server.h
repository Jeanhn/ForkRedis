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
        std::list<std::unique_ptr<Db>> *db_source_;
        Db *database_;

        std::deque<char> recv_buffer_;
        std::vector<json11::Json::array> recv_messages_;

        std::deque<char> send_buffer_;
        std::vector<json11::Json::array> send_messages_;

        auto Read() -> int;
        auto Send() -> int;

    public:
        void ShiftDB(int db_number);
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
        std::list<std::unique_ptr<Db>> *db_source_;
        std::unordered_map<int, std::unique_ptr<ClientInfo>> client_map_;
        std::vector<epoll_event> epoll_revents_;
        int listen_fd_;
        int epfd_;

    public:
        auto Wait(int timeout) -> std::vector<ClientInfo *>;
        void EnableSend(ClientInfo *);
        Server();
        ~Server();
    };

    class Handler
    {
    private:
        std::deque<std::pair<ClientInfo *, std::unique_ptr<CommandBase>>> command_que_;
        std::priority_queue<std::unique_ptr<Timer>, std::vector<std::unique_ptr<Timer>>,
                            decltype(&TimerGreater)>
            timer_que_{TimerGreater};

    public:
        void Push(ClientInfo *client, std::unique_ptr<CommandBase> cmd);
        void Push(std::unique_ptr<Timer> timer);
        void Handle();

        CLASS_DEFAULT_DECLARE(Handler);
    };

} // namespace rds

#endif