#include <database/aof.h>

namespace rds
{
    auto AOFLoad(std::deque<char> *source) -> std::list<std::unique_ptr<Db>>
    {
        auto rdb_str = PeekString(source, 3);
        if (rdb_str != "AOF")
        {
            throw std::runtime_error("RDBLoad: wrong file");
        }
        return {};
    }
} // namespace rds
