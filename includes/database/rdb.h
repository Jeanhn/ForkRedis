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
        const std::string name_{"REDIS"};
        const std::string db_version_{"0006"};
        std::list<std::unique_ptr<rds::Db>> *databases_;
        const char eof_ = 'e';
        const std::uint64_t check_sum_{0x12345678};

        std::size_t save_period_us_{1};

    public:
        auto Save() -> std::string;
        static auto Load(std::deque<char> *) -> std::list<std::unique_ptr<Db>>;
        void SetSaveFrequency(std::size_t second, std::size_t times);
        auto Period() const -> std::size_t;
        void Manage(std::list<std::unique_ptr<rds::Db>> *database_list);
        Rdb() = default;
        ~Rdb() = default;
    };

} // namespace rds

#endif