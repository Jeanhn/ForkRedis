#include <database/rdb.h>
#include <list>

namespace rds
{
    auto RDBLoad(std::deque<char> *source) -> std::list<std::unique_ptr<Db>>
    {
        Log("Loading rdb databases...");
        std::list<std::unique_ptr<Db>> ret;
        auto rdb_str = PeekString(source, 3);
        if (rdb_str != "RDB")
        {
            Log("RDB file loading error, return empty databases");
            return {};
        }

        while (!source->empty())
        {
            auto db = std::make_unique<Db>();
            db->Load(source);
            ret.push_back(std::move(db));
        }
        return ret;
    }

    void RDBSave(std::vector<std::string> database_sources, FileManager *dump_file)
    {
        dump_file->Truncate();
        dump_file->Write("RDB");
        for (auto &db : database_sources)
        {
            dump_file->Write(db);
        }
    }

} // namespace rds
