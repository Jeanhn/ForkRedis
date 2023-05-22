#include <event/muxer.h>
#include <sys/epoll.h>

namespace rds
{
    void Muxer::Add(Event *ev)
    {
        int fd = ev->GetFD();
        event_map_.insert({fd, std::move(ev)});
    }
    void Muxer::Del(Event *ev)
    {
        int fd = ev->GetFD();
        event_map_.erase(fd);
    }
    /*







     */

    SelectMuxer::SelectMuxer() : Muxer()
    {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
    }

    void SelectMuxer::Add(Event *ev)
    {
        int fd = ev->GetFD();
        if (ev->ReadCare())
        {
            FD_SET(fd, &rfds);
            read_fds.push_back(fd);
        }
        if (ev->WriteCare())
        {
            FD_SET(fd, &wfds);
            write_fds.push_back(fd);
        }
        Muxer::Add(std::move(ev));
    }

    void SelectMuxer::Del(Event *ev)
    {
        int fd = ev->GetFD();
        auto read_pos = std::find_if(read_fds.begin(), read_fds.end(), [fd](int f)
                                     { return f == fd; });
        auto write_pos = std::find_if(write_fds.begin(), write_fds.end(), [fd](int f)
                                      { return f == fd; });
        if (read_pos != read_fds.end())
        {
            read_fds.erase(read_pos);
        }
        if (write_pos != write_fds.end())
        {
            write_fds.erase(write_pos);
        }
        Muxer::Del(ev);
    }

    auto SelectMuxer::Poll(std::size_t timeout_us_) -> std::optional<std::vector<Event *>>
    {
        std::vector<Event *> ret;
        timeval *tv = nullptr;
        timeval tval;
        if (timeout_us_ > 0)
        {
            tv = &tval;
        }
        tval.tv_usec = timeout_us_;
        int n = select(event_map_.rbegin()->first, &rfds, &wfds, 0, tv);
        if (n == -1)
        {
            return {};
        }

        for (auto fd : read_fds)
        {
            if (FD_ISSET(fd, &rfds))
            {
                auto it = event_map_.find(fd);
                it->second->EnRead();
                ret.push_back(it->second);
            }
        }
        for (auto fd : write_fds)
        {
            if (FD_ISSET(fd, &wfds))
            {
                auto it = event_map_.find(fd);
                it->second->EnWrite();
                ret.push_back(it->second);
            }
        }
        return ret;
    }

    /*









     */

    void EpollMuxer::Add(Event *ev)
    {
        int epfd = epfd_.value();
        epoll_event ep_evt;
        ep_evt.data.fd = ev->GetFD();
        ep_evt.events = 0;
        if (ev->ReadCare())
        {
            ep_evt.events |= EPOLLIN;
        }
        if (ev->WriteCare())
        {
            ep_evt.events |= EPOLLOUT;
        }
        epoll_ctl(epfd, EPOLL_CTL_ADD, ev->GetFD(), &ep_evt);
        Muxer::Add(ev);
    }

    void EpollMuxer::Del(Event *ev)
    {
        int epfd = epfd_.value();
        epoll_ctl(epfd, EPOLL_CTL_DEL, ev->GetFD(), 0);
        Muxer::Del(ev);
    }

    auto EpollMuxer::Poll(std::size_t timeout_us) -> std::optional<std::vector<Event *>>
    {
        std::vector<Event *> ret;
        std::vector<epoll_event> ep_evts;
        ep_evts.resize(event_map_.size());

        int timeout_us_int = -1;
        if (timeout_us > 0)
        {
            timeout_us_int = static_cast<std::size_t>(timeout_us);
        }
        int n = epoll_wait(epfd_.value(), ep_evts.data(), 10240, timeout_us_int);
        if (n == -1)
        {
            return {};
        }
        for (auto &evt : ep_evts)
        {
            int fd = evt.data.fd;
            auto it = event_map_.find(fd);
            if (evt.events | EPOLLIN)
            {
                it->second->EnRead();
            }
            if (evt.events | EPOLLOUT)
            {
                it->second->EnWrite();
            }
            ret.push_back(it->second);
        }
        return ret;
    }
} // namespace rds
