#include <objects/hash.h>

namespace rds
{
    Hash::Hash(const Hash &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_map_ = lhs.data_map_;
    }

    Hash::Hash(Hash &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_map_ = std::move(rhs.data_map_);
    }

    Hash &Hash::operator=(const Hash &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_map_ = lhs.data_map_;
        return *this;
    }

    Hash &Hash::operator=(Hash &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_map_ = std::move(rhs.data_map_);
        return *this;
    }

    auto Hash::Get(const Str &key) -> Str
    {
        ReadGuard rg(latch_);
        auto it = data_map_.find(key);
        if (it == data_map_.end())
        {
            return {};
        }
        return it->second;
    }

    auto Hash::Exist(const Str &key) -> bool
    {
        ReadGuard rg(latch_);
        auto it = data_map_.find(key);
        return it != data_map_.end();
    }

    void Hash::Del(const Str &key)
    {
        WriteGuard wg(latch_);
        auto it = data_map_.find(key);
        if (it == data_map_.end())
        {
            return;
        }
        data_map_.erase(it);
    }

    auto Hash::Len() -> std::size_t
    {
        ReadGuard rg(latch_);
        return data_map_.size();
    }

    auto Hash::GetAll() -> std::vector<std::pair<Str, Str>>
    {
        std::vector<std::pair<Str, Str>> ret;
        ReadGuard rg(latch_);
        for (auto &element : data_map_)
        {
            ret.push_back(element);
        }
        return ret;
    }

    auto Hash::GetObjectType() const -> ObjectType
    {
        return ObjectType::HASH;
    }

    auto Hash::EncodeValue() const -> std::string
    {
        ReadGuard rg(latch_);
        std::string ret;
        ret.append(BitsToString(data_map_.size()));
        std::for_each(std::cbegin(data_map_), std::cend(data_map_),
                      [&ret](const decltype(data_map_)::value_type &kv) mutable
                      {
                          ret.append(kv.first.EncodeValue());
                          ret.append(kv.second.EncodeValue());
                      });
        return ret;
    }

    void Hash::DecodeValue(std::deque<char> *source)
    {
        WriteGuard wg(latch_);
        std::size_t len = PeekSize(source);
        for (std::size_t i = 0; i < len; i++)
        {
            Str k, v;
            k.DecodeValue(source);
            v.DecodeValue(source);
            data_map_.insert({k, v});
        }
    }

    auto Hash::IncrBy(const Str &key, int delta) -> std::string
    {
        ReadGuard rg(latch_);
        auto it = data_map_.find(key);
        if (it == data_map_.end())
        {
            return {};
        }
        return it->second.IncrBy(delta);
    }

    auto Hash::DecrBy(const Str &key, int delta) -> std::string
    {
        ReadGuard rg(latch_);
        auto it = data_map_.find(key);
        if (it == data_map_.end())
        {
            return {};
        }
        return it->second.DecrBy(delta);
    }

    void Hash::Set(const Str &key, Str value)
    {
        WriteGuard wg(latch_);
        auto it = data_map_.find(key);
        if (it != data_map_.end())
        {
            it->second = std::move(value);
            return;
        }
        data_map_.insert({key, std::move(value)});
    }

} // namespace rds
