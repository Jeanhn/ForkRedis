#ifndef __RDB_H__
#define __RDB_H__

#include <database/db.h>
#include <optional>
#include <util.h>
#include <list>
#include <database/disk.h>

namespace rds
{
    auto RDBLoad(std::deque<char> *source) -> std::list<std::unique_ptr<Db>>;

    void RDBSave(std::vector<std::string> database_sources, FileManager *dump_file);
} // namespace rds

#endif