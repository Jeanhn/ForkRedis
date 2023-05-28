#ifndef __AOF_H__
#define __AOF_H__

#include <database/db.h>
#include <database/rdb.h>

namespace rds
{
    auto AOFLoad(std::deque<char> *source) -> std::list<std::unique_ptr<Db>>;
} // namespace rds

#endif