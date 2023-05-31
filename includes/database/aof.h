#ifndef __AOF_H__
#define __AOF_H__

#include <database/db.h>
#include <database/rdb.h>

namespace rds
{
    void AOFLoad(const std::string ip, short port, std::deque<char> source);

    void AOFSave(std::vector<std::string> database_sources, FileManager *dump_file);

} // namespace rds

#endif