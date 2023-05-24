#include <server/server.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

namespace rds
{
    Server::Server()
    {
        sockaddr_in si;
        si.sin_addr.s_addr = inet_addr("127.0.0.1");
        si.sin_family = AF_INET;
        si.sin_port = htons(8080);
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
            auto it = client_map_.find(epoll_revents_[i].data.fd);
            if (it == client_map_.end())
            {
                std::cout << "Wait: valid client recv" << std::endl;
                epoll_ctl(epfd_, EPOLL_CTL_DEL, epoll_revents_[i].data.fd, 0);
                close(epoll_revents_[i].data.fd);
                continue;
            }
            if (it->first == listen_fd_)
            {
                int cfd = ::accept(listen_fd_, 0, 0);
                int fl = fcntl(cfd, F_GETFL);
                fcntl(fl, F_SETFL, fl | O_NONBLOCK);
                epoll_event epev;
                epev.data.fd = cfd;
                epev.events = EPOLLIN;
                epoll_ctl(epfd_, EPOLL_CTL_ADD, cfd, &epev);
                auto c = std::make_unique<ClientInfo>(cfd);
                c->db_source_ = db_source_;
                client_map_.insert({cfd, std::move(c)});
            }
            else
            {
                if (epoll_revents_[i].events & EPOLLIN)
                {
                    int nread = it->second->Read();
                    if (nread == 0 || nread == -1)
                    {
                        epoll_ctl(epfd_, EPOLL_CTL_DEL, epoll_revents_[i].data.fd, 0);
                        client_map_.erase(it);
                    }
                    else
                    {
                        ret.push_back(it->second.get());
                    }
                }
                else
                {
                    int nsend = it->second->Send();
                    if (nsend == -1)
                    {
                        epoll_ctl(epfd_, EPOLL_CTL_DEL, epoll_revents_[i].data.fd, 0);
                        client_map_.erase(it);
                    }
                    if (it->second->IsSendOut())
                    {
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
        command_que_.push_back({client, std::move(cmd)});
    }

    void Handler::Push(std::unique_ptr<Timer> timer)
    {
        if (!timer->valid_)
        {
            return;
        }
        timer_que_.push(std::move(timer));
    }

    void Handler::Handle()
    {
        while (!command_que_.empty())
        {
            auto it = command_que_.begin();
            auto res = it->second->Exec();
            it->first->Append(std::move(res));
            command_que_.pop_front();
        }

        std::size_t now_time_us = rds::UsTime();
        while (!timer_que_.empty() && timer_que_.top()->expire_time_us_ < now_time_us)
        {
            timer_que_.top()->Exec();
            timer_que_.pop();
        }
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
        auto beg = recv_buffer_.begin();
        std::vector<json11::Json::array> ret;
        do
        {
            while (*beg != '[' && beg != recv_buffer_.end())
            {
                beg++;
            }
            auto end = beg;
            while (*end != ']' && end != recv_buffer_.end())
            {
                end++;
            }
            if (beg != recv_buffer_.end() &&
                end != recv_buffer_.end() &&
                *beg == '[' && *end == ']')
            {
                end++;
                std::string data;
                std::copy(beg, end, std::back_inserter(data));
                recv_buffer_.erase(recv_buffer_.begin(), end);
                std::string err;
                json11::Json obj = json11::Json::parse(data, err);
                if (!err.empty())
                {
                    continue;
                }
                ret.push_back(obj.array_items());
            }
            else
            {
                break;
            }
        } while (1);
        return ret;
    }

    void ClientInfo::ShiftDB(int db_number)
    {
        auto pos = std::find_if(db_source_->cbegin(), db_source_->cend(), [db_number](const std::unique_ptr<Db> &db)
                                { return db->Number() == db_number; });
        if (pos == db_source_->cend())
        {
            database_ = nullptr;
            return;
        }
        database_ = pos->get();
    }

    auto ClientInfo::GetDB() -> Db *
    {
        return database_;
    }

    ClientInfo::ClientInfo(int fd) : fd_(fd), db_source_(nullptr), database_(nullptr)
    {
    }

    ClientInfo::~ClientInfo()
    {
        close(fd_);
    }

} // namespace rds
