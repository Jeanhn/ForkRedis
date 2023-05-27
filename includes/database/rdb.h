#ifndef __RDB_H__
#define __RDB_H__

#include <database/db.h>
#include <optional>
#include <util.h>
#include <list>

namespace rds
{
    class Rdb
    {
#ifdef NDEBUG
    private:
#else
    public:
#endif
        std::function<std::string()> generator_;
        std::size_t save_period_us_{1};

    public:
        static auto Load(std::deque<char> *) -> std::list<std::unique_ptr<Db>>;
        void SetSaveFrequency(std::size_t second, std::size_t times);
        auto Period() const -> std::size_t;
        Rdb(std::function<std::string()> generator);
        Rdb() = default;
        ~Rdb() = default;
    };

} // namespace rds

#endif