#include <server/server.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <thread>
#include <condition_variable>

namespace rds
{
    Server::Server(const char *ip, short port)
    {
        sockaddr_in si;
        si.sin_addr.s_addr = inet_addr(ip);
        si.sin_family = AF_INET;
        si.sin_port = htons(port);
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        Assert(listen_fd_ != -1, "listenfd");
        int ret = bind(listen_fd_, reinterpret_cast<sockaddr *>(&si), sizeof(si));
        Assert(ret != -1, "bind");
        listen(listen_fd_, 200);
        int fl = fcntl(listen_fd_, F_GETFL);
        fcntl(fl, F_SETFL, fl | O_NONBLOCK);

        epfd_ = epoll_create(200);
        Assert(epfd_ != -1, "epollfd");
        epoll_event levt;
        levt.data.fd = listen_fd_;
        levt.events = EPOLLIN;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, listen_fd_, &levt);

        Log("Server runs successfully on: ", ip, ':', port);

        // Log("Loading databases...");
        // auto dbfile = file_manager_.LoadAndExport();
        // file_manager_.Truncate();

        // databases_ = rds::Rdb::Load(&dbfile);

        // if (databases_.empty())
        // {
        //     Log("Create a default database");
        //     databases_.push_back(std::make_unique<Db>());
        // }
    }

    Server::~Server()
    {
        close(listen_fd_);
    }

    auto Server::Wait(int timeout) -> std::vector<ClientInfo *>
    {
        epoll_revents_.resize(client_map_.size() + 1);
        int n = epoll_wait(epfd_, epoll_revents_.data(),
                           epoll_revents_.size(), timeout);
        if (n == -1)
        {
            return {};
        }
        std::vector<ClientInfo *> ret;
        for (int i = 0; i < n; i++)
        {
            if (epoll_revents_[i].data.fd == listen_fd_)
            {
#ifndef NDEBUG
                std::cout << "New client come" << std::endl;
#endif
                int cfd = ::accept(listen_fd_, 0, 0);
                int fl = fcntl(cfd, F_GETFL);
                fcntl(fl, F_SETFL, fl | O_NONBLOCK);
                epoll_event epev;
                epev.data.fd = cfd;
                epev.events = EPOLLIN;
                epoll_ctl(epfd_, EPOLL_CTL_ADD, cfd, &epev);
                auto c = std::make_unique<ClientInfo>(cfd);
                // c->db_source_ = &databases_;
                client_map_.insert({cfd, std::move(c)});
            }
            else
            {
                auto it = client_map_.find(epoll_revents_[i].data.fd);
                if (it == client_map_.end())
                {
#ifndef NDEBUG
                    Log("Wait: invalid client recv");
#endif
                    epoll_ctl(epfd_, EPOLL_CTL_DEL, epoll_revents_[i].data.fd, 0);
                    close(epoll_revents_[i].data.fd);
                    continue;
                }
                if (epoll_revents_[i].events & EPOLLIN)
                {
#ifndef NDEBUG
                    Log("Client message recv");
#endif
                    int nread = it->second->Read();
                    if (nread == 0 || nread == -1)
                    {
#ifndef NDEBUG
                        Log("Closed or invalid read");
#endif
                        epoll_ctl(epfd_, EPOLL_CTL_DEL, epoll_revents_[i].data.fd, 0);
                        client_map_.erase(it);
                    }
                    else
                    {
#ifndef NDEBUG
                        Log("Valid read");
#endif
                        ret.push_back(it->second.get());
                    }
                }
                else
                {
#ifndef NDEBUG
                    Log("Server message send");
#endif
                    int nsend = it->second->Send();
                    if (nsend == -1)
                    {
#ifndef NDEBUG
                        Log("Inalid send");
#endif
                        epoll_ctl(epfd_, EPOLL_CTL_DEL, epoll_revents_[i].data.fd, 0);
                        client_map_.erase(it);
                    }
                    else if (it->second->IsSendOut())
                    {
#ifndef NDEBUG
                        Log("Valid send");
#endif
                        epoll_event epev;
                        epev.data.fd = epoll_revents_[i].data.fd;
                        epev.events = EPOLLIN;
                        epoll_ctl(epfd_, EPOLL_CTL_MOD, epoll_revents_[i].data.fd, &epev);
                    }
                }
            }
        }
        return ret;
    }

    /*








     */
    void Handler::Push(ClientInfo *client, std::unique_ptr<CommandBase> cmd)
    {
        if (!cmd->valid_)
        {
            return;
        }
        cmd->cli_ = client;
        cmd_que_.Push(client, std::move(cmd));
    }

    void Handler::Push(std::unique_ptr<Timer> timer)
    {
        if (!timer->valid_)
        {
            return;
        }
        tmr_que_.Push(std::move(timer));
    }

    void Handler::ExecCommand()
    {
        while (running_)
        {
            auto pair = cmd_que_.BlockPop();
            auto respond = pair.second->Exec();
            pair.first->Append(respond);
        }
    }

    void Handler::ExecTimer()
    {
        while (running_)
        {
            auto tmr = tmr_que_.BlockPop();
            tmr->Exec();
        }
    }

    void Handler::Run()
    {
        running_ = true;
        // std::thread exec_cmd_(, this);
        // std::thread exec_tmr_(, this);
        // workers_.push_back(std::move(exec_cmd_));
        // workers_.push_back(std::move(exec_tmr_));
    }

    void Server::EnableSend(ClientInfo *client)
    {
        epoll_event epev;
        epev.data.fd = client->fd_;
        epev.events = EPOLLOUT;
        epoll_ctl(epfd_, EPOLL_CTL_MOD, client->fd_, &epev);
    }

    /*




     */
    auto ClientInfo::Read() -> int
    {
        char buf[65535];
        int total_n = 0;
        int n;
        do
        {
            n = read(fd_, buf, 65535);
            total_n += n;
            if (n == -1)
            {
                return -1;
            }
            if (n == 0)
            {
                return 0;
            }
            std::copy(std::cbegin(buf), std::cbegin(buf) + n, std::back_inserter(recv_buffer_));
#ifndef NDEBUG
            Log("Read");
            std::copy(recv_buffer_.cbegin(), recv_buffer_.cend(), std::ostreambuf_iterator<char>(std::cout));
            std::cout << std::endl;
#endif
        } while (n == 65535);
        return total_n;
    }

    auto ClientInfo::Send() -> int
    {
        if (send_buffer_.empty())
        {
            return 0;
        }
        std::vector<char> buf;
        buf.reserve(send_buffer_.size() + 10);
        std::copy(send_buffer_.cbegin(), send_buffer_.cend(), std::back_inserter(buf));
        int n;
        n = write(fd_, buf.data(), buf.size());
#ifndef NDEBUG
        Log("Send");
        std::copy(buf.cbegin(), buf.cend(), std::ostreambuf_iterator<char>(std::cout));
        std::cout << std::endl;
#endif
        if (n == 0 || n == -1)
        {
            return n;
        }
        send_buffer_.erase(send_buffer_.begin(), send_buffer_.begin() + n);
        return n;
    }

    void ClientInfo::Append(json11::Json::array to_send_message)
    {
        json11::Json obj(std::move(to_send_message));
        auto str = obj.dump();
        std::copy(str.cbegin(), str.cend(), std::back_inserter(send_buffer_));
    }

    auto ClientInfo::IsSendOut() -> bool
    {
        return send_buffer_.empty();
    }

    auto ClientInfo::ExportMessages() -> std::vector<json11::Json::array>
    {
        std::vector<json11::Json::array> ret;
        do
        {
            auto beg = std::find(recv_buffer_.begin(), recv_buffer_.end(), '[');
            auto end = std::find(recv_buffer_.begin(), recv_buffer_.end(), ']');
            if (beg == recv_buffer_.end() ||
                end == recv_buffer_.end() ||
                std::distance(beg, end) <= 0)
            {
                break;
            }
            end++;
            std::string element, err;
            std::copy(beg, end, std::back_inserter(element));
            json11::Json req = json11::Json::parse(element, err);
            ret.push_back(req.array_items());
            recv_buffer_.erase(recv_buffer_.begin(), end);
        } while (1);
        return ret;
    }

    void ClientInfo::ShiftDB(Db *database)
    {
        database_ = database;
    }

    auto ClientInfo::GetDB() -> Db *
    {
        return database_;
    }

    ClientInfo::ClientInfo(int fd) : fd_(fd), database_(nullptr)
    {
    }

    ClientInfo::~ClientInfo()
    {
        close(fd_);
    }

} // namespace rds
