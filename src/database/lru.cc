#include <database/lru.h>

namespace rds
{

    auto LRU::Access(const std::string &key, std::size_t size) -> std::size_t
    {

        map_lock_.ReadLock();
        auto it = map_.find(key);
        std::lock_guard lg(it->second.mtx_);
        if (it != map_.end())
        {
            WriteGuard wg(rank_mtx_);
            std::string val = std::move(it->second.itr->first);
            size_ -= it->second.itr->second;
            rank_.erase(it->second.itr);
            rank_.push_front({std::move(val), size});
            size_ += size;
            it->second.itr = rank_.begin();
            map_lock_.ReadUnlock();
            return size_.load();
        }

        map_lock_.UPGrade();
        WriteGuard wg(rank_mtx_);
        rank_.push_front({key, size});
        std::pair<std::string, Node> p;
        p.second.itr = rank_.begin();
        map_.insert(std::move(p));
        size_ += size;
        map_lock_.WriteUnlock();
        return size_.load();
    }

    void LRU::Remove(const std::string &key)
    {
        map_lock_.WriteLock();
        auto it = map_.find(key);
        if (it == map_.end())
        {
            map_lock_.WriteUnlock();
            return;
        }
        std::lock_guard lg(it->second.mtx_);
        WriteGuard wg(rank_mtx_);
        size_ -= it->second.itr->second;
        rank_.erase(it->second.itr);
        map_.erase(it);
        map_lock_.WriteUnlock();
        return;
    }

    auto LRU::Evict() -> std::string
    {
        map_lock_.WriteLock();
        WriteGuard wg(rank_mtx_);
        auto ret = rank_.back().first;
        map_.erase(rank_.back().first);
        rank_.pop_back();
        map_lock_.WriteUnlock();
        return ret;
    }

    LRU::LRU(const RedisConf &conf) : conf_(conf),
                                      mem_limit_bytes_(conf.mem_size_mbytes_ > 4096 ? conf.mem_size_mbytes_ : 4096)
    {
        mem_limit_bytes_ = (mem_limit_bytes_ < 1024 * 8 ? mem_limit_bytes_ : 1024 * 8);
    }

} // namespace rds
