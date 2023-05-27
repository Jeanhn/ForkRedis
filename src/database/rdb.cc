#include <database/rdb.h>
#include <list>

namespace rds
{
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

} // namespace rds
