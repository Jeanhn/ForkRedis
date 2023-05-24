#include <database/rdb.h>
#include <list>

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

    auto Rdb::Load(std::deque<char> *source) -> std::list<std::unique_ptr<Db>>
    {
        if (source->empty())
        {
            return {};
        }
        PeekString(source, 5);
        PeekString(source, 4);

        std::list<std::unique_ptr<Db>> ret;
        while (source->front() != 'e')
        {
            auto db = std::make_unique<Db>();
            db->Load(source);
            ret.push_back(std::move(db));
        }
        return ret;
    }

    void Rdb::SetSave(std::size_t second, std::size_t times)
    {
        save_period_us_ = second * 1000'000 / times;
    }

} // namespace rds
