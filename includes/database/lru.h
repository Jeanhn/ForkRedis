#ifndef __LRU_H__
#define __LRU_H__

#include <util.h>
#include <list>
#include <unordered_map>

namespace rds
{
    class UPGradeLock
    {
        std::mutex mtx_;
        std::shared_mutex mu_;

    public:
        void ReadLock()
        {
            std::lock_guard lg(mtx_);
            mu_.lock_shared();
        }
        void ReadUnlock()
        {
            std::lock_guard lg(mtx_);
            mu_.unlock_shared();
        }
        void UPGrade()
        {
            std::lock_guard lg(mtx_);
            mu_.unlock_shared();
            mu_.lock();
        }
        void WriteLock()
        {
            std::lock_guard lg(mtx_);
            mu_.lock();
        }
        void WriteUnlock()
        {
            std::lock_guard lg(mtx_);
            mu_.unlock();
        }
    };
    class LRU
    {
    private:
        const RedisConf conf_;
        std::size_t mem_limit_bytes_;
        std::atomic_uint32_t size_;

        mutable std::shared_mutex rank_mtx_;
        std::list<std::pair<std::string, std::size_t>> rank_;

        struct Node
        {
            /* data */
            decltype(rank_)::iterator itr;
            mutable std::mutex mtx_;
            Node() = default;
            Node(const Node &n)
            {
                std::lock_guard lg(n.mtx_);
                itr = n.itr;
            }
            Node(Node &&n)
            {
                std::lock_guard lg(n.mtx_);
                itr = std::move(n.itr);
            }
            Node &operator=(const Node &n)
            {
                std::lock_guard lg(n.mtx_);
                itr = n.itr;
                return *this;
            }
            Node &operator=(Node &&n)
            {
                std::lock_guard lg(n.mtx_);
                itr = std::move(n.itr);
                return *this;
            }
            ~Node() = default;
        };

        UPGradeLock map_lock_;
        std::unordered_map<std::string, Node> map_;

    public:
        auto Access(const std::string &key, std::size_t size) -> std::size_t;
        void Remove(const std::string &key);
        auto Evict() -> std::string;
        auto IsFull() -> bool { return size_.load() >= mem_limit_bytes_; }
        LRU(const RedisConf &);
    };
}

#endif