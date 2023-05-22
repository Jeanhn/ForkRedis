#ifndef __EVENT_H__
#define __EVENT_H__

#include <util.h>
#include <functional>
#include <unordered_map>

namespace rds
{
    class Event
    {
    protected:
        std::array<int, 3> care_;
        std::array<int, 3> trriger;

        std::optional<int> fd_;
        std::unordered_map<std::string, std::vector<std::function<void(Event *)>>> call_backs_;
        auto Read(std::string *) -> int;
        auto Wirte(const std::string &) -> int;
        auto AddCallBack(std::string, std::function<void(Event *)>);

    public:
        auto ReadCare() -> bool;
        auto WriteCare() -> bool;
        auto TimeCare() -> bool;
        void EnRead();
        void EnWrite();
        void EnTime();
        auto Readable() -> bool;
        auto Writeable() -> bool;
        auto Timeable() -> bool;

        auto GetFD() -> int;
        virtual void TrigerCallBack() = 0;

        Event() = delete;
        Event(const Event &) = delete;
        Event &operator=(const Event &) = delete;

        Event(Event &&) = default;
        Event &operator=(Event &&) = default;

        virtual ~Event();

        Event(int fd);
    };

    class FileEvent : public Event
    {
    private:
    public:
        void TrigerCallBack();
        void AddRead();
        void AddWrite();
    };

    class TimeEvent : public Event
    {
    private:
    public:
        void TrigerCallBack();
        void AddTime();
    };

} // namespace rds

#endif