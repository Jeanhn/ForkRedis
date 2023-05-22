#ifndef __MUXER_H__
#define __MUXER_H__

#include <util.h>
#include <event/event.h>
#include <map>
#include <set>

namespace rds
{
    class Muxer
    {
    protected:
        std::map<int, Event *> event_map_;

    public:
        virtual void Add(Event *);
        virtual void Del(Event *);
        virtual auto Poll(std::size_t = 0) -> std::optional<std::vector<Event *>>;

        CLASS_DECLARE_uncopyable(Muxer);
        Muxer() = default;
    };

    class SelectMuxer : public Muxer
    {
    private:
        fd_set rfds;
        std::vector<int> read_fds;
        fd_set wfds;
        std::vector<int> write_fds;

    public:
        SelectMuxer();
        void Add(Event *);
        void Del(Event *);
        auto Poll(std::size_t = 0) -> std::optional<std::vector<Event *>>;
    };

    class EpollMuxer : public Muxer
    {
    private:
        std::optional<int> epfd_;

    public:
        void Add(Event *);
        void Del(Event *);
        auto Poll(std::size_t = 0) -> std::optional<std::vector<Event *>>;
    };

} // namespace rds

#endif