#ifndef __LRU_H__
#define __LRU_H__

#include <util.h>
#include <map>
#include <list>
#include <objects/str.h>

namespace rds
{
    class StrLRU
    {
    private:
        std::map<std::size_t, Str> data_map_;
        std::map<Str, decltype(data_map_)::iterator, decltype(&StrLess)> index_map_{StrLess};

    public:
        void Set(const Str &key, std::size_t time_stamp)
        {
            auto it = index_map_.find(key);
            if (it == index_map_.end())
            {
                auto idx = data_map_.insert({time_stamp, key});
                index_map_.insert({key, idx.first});
                return;
            }
            data_map_.erase(it->second);
            auto idx = data_map_.insert({time_stamp, key});
            it->second = idx.first;
        }

        void Del(const Str &key)
        {
            auto it = index_map_.find(key);
            if (it == index_map_.end())
            {
                return;
            }
            data_map_.erase(it->second);
            index_map_.erase(it);
        }

        auto Evict() -> Str
        {
            if (data_map_.empty())
            {
                return {};
            }
            auto it = data_map_.begin();
            if (it->first > UsTime())
            {
                return {};
            }
            Str ret = std::move(it->second);
            data_map_.erase(it);
            index_map_.erase(ret);
            return ret;
        }

        auto Empty() -> bool
        {
            return data_map_.empty();
        }

        auto Top() -> Str
        {
            return data_map_.cbegin()->second;
        }

        CLASS_DEFAULT_DECLARE(StrLRU)
    };

} // namespace rds

#endif