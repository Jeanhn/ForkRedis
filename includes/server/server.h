#ifndef __SERVER_H__
#define __SERVER_H__

#include <util.h>
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
        friend void AOFLoad(const std::string &ip, short port, std::deque<char> *source);
#ifdef NDEBUG
    private:
#else
    public:
#endif
        int fd_;
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
        void EnableRead();
        void Logout();
        ClientInfo(Server *server = nullptr, int fd = -1);
        ~ClientInfo();
        ClientInfo(const ClientInfo &) = delete;
        ClientInfo(ClientInfo &&) = delete;
    };

    class Server
    {
    private:
        std::shared_mutex latch_;
        std::unordered_map<int, std::shared_ptr<ClientInfo>> client_map_;
        std::vector<epoll_event> epoll_revents_;
        int listen_fd_;
        int epfd_;

    public:
        auto Wait(int timeout) -> std::vector<std::pair<std::shared_ptr<ClientInfo>, int>>;
        void EnableSend(ClientInfo *client);
        void EnableRead(ClientInfo *client);
        void RemoveCli(int cli_fd);
        auto ReadCli(int cli_fd) -> int;
        auto SendCli(int cli_fd) -> int;
        Server(const char *ip = "127.0.0.1", short port = 8080);
        ~Server();
    };

} // namespace rds

#endif