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
    private:
        const std::string name_{"REDIS"};
        const std::string db_version_{"0006"};
        std::vector<Db *> databases_;
        const char eof_ = 'e';
        const std::uint64_t check_sum_{0x12345678};

        std::size_t save_period_us_;

    public:
        auto Save() -> std::string;
        static auto Load(std::deque<char> *) -> std::list<std::unique_ptr<Db>>;
        void SetSave(std::size_t second, std::size_t times);
        auto Period() const -> std::size_t;
        Rdb() = default;
        ~Rdb() = default;
    };

} // namespace rds

#endif