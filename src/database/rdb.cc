#include <database/rdb.h>
#include <list>

namespace rds
{
    auto Rdb::Save() -> std::string
    {
        std::string ret;
        ret.append(name_);
        ret.append(db_version_);
        for (auto &db : *databases_)
        {
            ret.append(db->Save());
        }
        ret.push_back(eof_);
        ret.append(BitsToString(check_sum_));
        return ret;
    }

    auto Rdb::Load(std::deque<char> *source) -> std::list<std::unique_ptr<Db>>
    {
        std::list<std::unique_ptr<Db>> ret;
        if (source->empty())
        {
            auto db = std::make_unique<Db>();
            ret.push_back(std::move(db));
            return ret;
        }

        PeekString(source, 5);
        PeekString(source, 4);

        while (source->front() != 'e')
        {
            auto db = std::make_unique<Db>();
            db->Load(source);
            ret.push_back(std::move(db));
        }
        return ret;
    }

    void Rdb::SetSaveFrequency(std::size_t second, std::size_t times)
    {
        save_period_us_ = second * 1000'000 / times;
    }

    auto Rdb::Period() const -> std::size_t
    {
        return save_period_us_;
    }

    void Rdb::Manage(std::list<std::unique_ptr<rds::Db>> *database_list)
    {
        databases_ = database_list;
    }

} // namespace rds
