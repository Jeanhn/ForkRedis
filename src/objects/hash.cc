#include <objects/hash.h>

namespace rds
{

    template <typename T,
              typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
    void Hash::Set(const Str &key, T &&value)
    {
        auto it = data_map_.find(key);
        if (it != data_map_.end())
        {
            it->second = std::forward<T>(value);
            return;
        }
        data_map_.insert({key, std::forward<T>(value)});
    }

    auto Hash::Get(const Str &key) -> Str
    {
        auto it = data_map_.find(key);
        if (it == data_map_.end())
        {
            return {};
        }
        return it->second;
    }

    auto Hash::Exist(const Str &key) -> bool
    {
        auto it = data_map_.find(key);
        if (it == data_map_.end())
        {
            return false;
        }
        return true;
    }

    void Hash::Del(const Str &key)
    {
        auto it = data_map_.find(key);
        if (it == data_map_.end())
        {
            return;
        }
        data_map_.erase(it);
    }

    auto Hash::Len() -> std::size_t
    {
        return data_map_.size();
    }

    auto Hash::GetAll() -> std::vector<std::pair<Str, Str>>
    {
        std::vector<std::pair<Str, Str>> ret;
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
        std::string ret;
        ret.append(BitsToString(data_map_.size()));
        std::for_each(std::cbegin(data_map_), std::cend(data_map_),
                      [&ret](const decltype(data_map_)::value_type &kv)
                      {
                          ret.append(kv.first.EncodeValue());
                          ret.append(kv.second.EncodeValue());
                      });
    }

    void Hash::DecodeValue(std::deque<char> &source)
    {
        std::size_t len = PeekSize(source);
        for (std::size_t i = 0; i < len; i++)
        {
            Str k, v;
            k.DecodeValue(source);
            v.DecodeValue(source);
            data_map_.insert({k, v});
        }
    }

} // namespace rds
