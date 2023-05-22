#ifndef __RDB_H__
#define __RDB_H__

#include <database/db.h>
#include <optional>

namespace rds
{
    class Rdb
    {
    private:
        const std::string name_{"REDIS"};
        const std::string db_version_{"0006"};
        std::vector<Db *> databases_;
        std::map<Db *, std::size_t> frequency_map_;
        const char eof_ = '0';
        std::uint64_t check_sum_;

    public:
        auto Save() -> std::string;
        auto Load(std::deque<char> *) -> std::vector<std::unique_ptr<Db>>;
        void SetSave(std::size_t second, std::size_t times, int db_number);
        Rdb();
        ~Rdb();
    };

} // namespace rds

#endif