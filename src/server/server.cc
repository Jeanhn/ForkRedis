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
    void SetNonBlock(int fd)
    {
        int fl = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, fl | O_NONBLOCK | O_NDELAY);
    }
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
        SetNonBlock(listen_fd_);

        epfd_ = epoll_create(200);
        Assert(epfd_ != -1, "epollfd");
        epoll_event levt;
        levt.data.fd = listen_fd_;
        levt.events = EPOLLIN;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, listen_fd_, &levt);

        Log("Server runs successfully on: ", ip, ':', port);
    }

    Server::~Server()
    {
        close(listen_fd_);
    }

    auto Server::ReadCli(int cli_fd) -> int
    {
        ReadGuard rg(latch_);
        auto it = client_map_.find(cli_fd);
        if (it == client_map_.end())
        {
            epoll_ctl(epfd_, EPOLL_CTL_DEL, cli_fd, 0);
            close(cli_fd);
            return -1;
        }
        return it->second->Read();
    }
    auto Server::SendCli(int cli_fd) -> int
    {
        ReadGuard rg(latch_);
        auto it = client_map_.find(cli_fd);
        if (it == client_map_.end())
        {
            epoll_ctl(epfd_, EPOLL_CTL_DEL, cli_fd, 0);
            close(cli_fd);
            return -1;
        }
        return it->second->Send();
    }

    void Server::RemoveCli(int cli_fd)
    {
        WriteGuard wg(latch_);
        epoll_ctl(epfd_, EPOLL_CTL_DEL, cli_fd, 0);
        auto it = client_map_.find(cli_fd);
        if (it == client_map_.end())
        {
            return;
        }
        client_map_.erase(it);
    }

    auto Server::Wait(int timeout) -> std::vector<std::pair<std::shared_ptr<ClientInfo>, int>>
    {
        epoll_revents_.resize(client_map_.size() + 1);
        int n = epoll_wait(epfd_, epoll_revents_.data(),
                           epoll_revents_.size(), timeout);
        if (n == -1)
        {
            return {};
        }
        std::vector<std::pair<std::shared_ptr<ClientInfo>, int>> ret;
        for (int i = 0; i < n; i++)
        {
            if (epoll_revents_[i].data.fd == listen_fd_)
            {
                int cfd = ::accept(listen_fd_, 0, 0);
                SetNonBlock(cfd);
                epoll_event epev;
                epev.data.fd = cfd;
                epev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd_, EPOLL_CTL_ADD, cfd, &epev);
                auto c = std::make_shared<ClientInfo>(this, cfd);
                WriteGuard rg(latch_);
                client_map_.insert({cfd, std::move(c)});
            }
            else
            {
                ReadGuard rg(latch_);
                auto it = client_map_.find(epoll_revents_[i].data.fd);
                if (it == client_map_.end())
                {
                    epoll_ctl(epfd_, EPOLL_CTL_DEL, epoll_revents_[i].data.fd, 0);
                    close(epoll_revents_[i].data.fd);
                    continue;
                }
                if (epoll_revents_[i].events & EPOLLIN)
                {
                    ret.push_back({it->second, EPOLLIN});
                }
                else
                {
                    ret.push_back({it->second, EPOLLOUT});
                }
            }
        }
        return ret;
    }

    /*








     */

    void Server::EnableSend(ClientInfo *client)
    {
        epoll_event epev;
        epev.data.fd = client->GetFD();
        epev.events = EPOLLOUT | EPOLLET;
        epoll_ctl(epfd_, EPOLL_CTL_MOD, client->GetFD(), &epev);
    }

    void Server::EnableRead(ClientInfo *client)
    {
        epoll_event epev;
        epev.data.fd = client->GetFD();
        epev.events = EPOLLIN | EPOLLET;
        epoll_ctl(epfd_, EPOLL_CTL_MOD, client->GetFD(), &epev);
    }

    /*




     */
    auto ClientInfo::Read() -> int
    {
        char buf[65535];
        int total_n = 0;
        int n;
        WriteGuard wg(latch_);
        do
        {
            n = recv(fd_, buf, 65535, 0);
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
        } while (n == 65535);
        return total_n;
    }

    auto ClientInfo::Send() -> int
    {
        WriteGuard wg(latch_);
        if (send_buffer_.empty())
        {
            return 0;
        }
        std::vector<char> buf;
        buf.reserve(send_buffer_.size() + 10);
        std::copy(send_buffer_.cbegin(), send_buffer_.cend(), std::back_inserter(buf));
        int n;
        n = send(fd_, buf.data(), buf.size(), 0);
        if (n == 0 || n == -1)
        {
            return n;
        }
        send_buffer_.erase(send_buffer_.begin(), send_buffer_.begin() + n);
        return n;
    }

    void ClientInfo::Append(json11::Json::array to_send_message)
    {
        WriteGuard wg(latch_);
        json11::Json obj(std::move(to_send_message));
        auto str = obj.dump();
        std::copy(str.cbegin(), str.cend(), std::back_inserter(send_buffer_));
    }

    auto ClientInfo::IsSendOut() -> bool
    {
        ReadGuard rg(latch_);
        return send_buffer_.empty();
    }

    auto ClientInfo::ExportMessages() -> std::vector<json11::Json::array>
    {
        WriteGuard wg(latch_);
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

    void ClientInfo::SetDB(Db *database)
    {
        WriteGuard wg(latch_);
        database_ = database;
    }

    auto ClientInfo::GetDB() -> Db *
    {
        ReadGuard rg(latch_);
        return database_;
    }

    auto ClientInfo::GetFD() -> int
    {
        ReadGuard rg(latch_);
        return fd_;
    }

    void ClientInfo::EnableSend()
    {
        decltype(this) ths;
        {
            ReadGuard rg(latch_);
            ths = this;
        }
        server_->EnableSend(ths);
    }

    void ClientInfo::Logout()
    {
        server_->RemoveCli(GetFD());
    }

    void ClientInfo::EnableRead()
    {
        decltype(this) ths;
        {
            ReadGuard rg(latch_);
            ths = this;
        }
        server_->EnableRead(ths);
    }

    ClientInfo::ClientInfo(Server *server, int fd) : fd_(fd), server_(server), database_(nullptr)
    {
    }

    ClientInfo::~ClientInfo()
    {
        close(fd_);
    }

} // namespace rds
