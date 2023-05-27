#ifndef __AOF_H__
#define __AOF_H__

#include <database/db.h>
#include <database/rdb.h>

namespace rds
{
    class Aof
    {
    private:
    public:
        static auto Load(std::deque<char> *) -> std::list<std::unique_ptr<Db>>;
    };

} // namespace rds

#endif