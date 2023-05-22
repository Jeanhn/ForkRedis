#include <database/rdb.h>

namespace rds
{
    auto Rdb::Save() -> std::string
    {
        std::string ret;
        ret.append(name_);
        ret.append(db_version_);
        for (auto &db : databases_)
        {
            ret.append(db->Save());
        }
        ret.push_back(eof_);
        ret.append(BitsToString(check_sum_));
        return ret;
    }

    auto Rdb::Load(std::deque<char> *source) -> std::vector<std::unique_ptr<Db>>
    {
        auto name = PeekString(source, 5);
        auto version = PeekString(source, 4);

        std::vector<std::unique_ptr<Db>> ret;
        while (source->front() != eof_)
        {
            auto db = std::make_unique<Db>();
            db->Load(source);
            ret.push_back(std::move(db));
            databases_.push_back(db.get());
        }
        return ret;
    }

} // namespace rds
