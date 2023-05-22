#ifndef __AOF_H__
#define __AOF_H__

#include <database/db.h>
#include <database/rdb.h>

namespace rds
{
    class Aof
    {
    private:
        std::unordered_map<Str, std::vector<std::string>, decltype(&StrHash)> aof_buffer_{0xff, StrHash};
        std::vector<Db *> databases_;
        void Optimize(const Str &);
        void OptimizeAll();

    public:
        void Load(std::deque<char> *);
        void Append(const Str &, std::string);
        void ReWrite();
    };

} // namespace rds

#endif