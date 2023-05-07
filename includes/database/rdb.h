#ifndef __RDB_H__
#define __RDB_H__

#include <database/db.h>
#include <optional>

namespace rds
{
    struct KeyValuePair
    {
        std::optional<char> expire_time_ms_;
        std::size_t ms_;
        char obj_type_;
        std::string key_;
        std::string value_;
    };

    struct DataBaseEntry
    {
        const char select_flag_ = 's';
        char db_number_;
        std::vector<KeyValuePair> key_value_pairs_;
    };

    class Rdb
    {
    private:
        const std::string name_{"REDIS"};
        const std::string db_version_{"0006"};
        std::vector<DataBaseEntry> databases_;
        const char eof_ = '0';
        std::uint64_t check_sum_;

    private:
        void Save();
        void BGSave();

    public:
        void CheckSave(std::size_t, int); // int times per size_t seconds
        Rdb(/* args */);
        ~Rdb();
    };

} // namespace rds

#endif