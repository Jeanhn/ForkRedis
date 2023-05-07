#ifndef __UTIL_H__
#define __UTIL_H__

#include <cassert>
#include <utility>
#include <tuple>
#include <type_traits>
#include <memory>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <atomic>
#include <algorithm>
#include <iterator>
#include <unistd.h>
#include <sys/time.h>
#include <string>
#include <cstring>
#include <cassert>
#include <deque>

#define CLASS_DEFAULT_DECLARE(name)                 \
    name() = default;                               \
    ~name() = default;                              \
    name(const name &) = default;                   \
    name(name &&) noexcept = default;               \
    auto operator=(const name &)->name & = default; \
    auto operator=(name &&) noexcept -> name & = default;

#define CLASS_DECLARE_without_destructor(name)      \
    name() = default;                               \
    name(const name &) = default;                   \
    name(name &&) noexcept = default;               \
    auto operator=(const name &)->name & = default; \
    auto operator=(name &&) noexcept -> name & = default;

#define CLASS_DECLARE_without_constructor(name)     \
    ~name() = default;                              \
    name(const name &) = default;                   \
    name(name &&) noexcept = default;               \
    auto operator=(const name &)->name & = default; \
    auto operator=(name &&) noexcept -> name & = default;

namespace rds
{
    class ReadGuard
    {
    private:
        std::shared_mutex &latch_;

    public:
        ReadGuard(std::shared_mutex &latch) : latch_(latch)
        {
            latch_.lock_shared();
        };
        ~ReadGuard()
        {
            latch_.unlock_shared();
        }
    };

    class WriteGuard
    {
    private:
        std::shared_mutex &latch_;

    public:
        WriteGuard(std::shared_mutex &latch) : latch_(latch) { latch_.lock(); };
        ~WriteGuard()
        {
            latch_.unlock();
        }
    };

    std::size_t UsTime(void)
    {
        struct timeval tv;
        std::size_t ust;

        gettimeofday(&tv, NULL);
        ust = ((std::size_t)tv.tv_sec) * 1000000;
        ust += tv.tv_usec;
        return ust;
    }

    std::size_t MsTime(void)
    {
        return UsTime() / 1000;
    }

    template <typename BitType>
    auto BitsToString(BitType data) -> std::string
    {
        std::string ret;
        char buf[sizeof(BitType) + 2];
        memcpy(buf, &data, sizeof(BitType));
        ret.append(std::cbegin(buf), std::cbegin(buf) + sizeof(BitType));
        return ret;
    }

    auto PeekInt(std::deque<char> &source) -> int
    {
        int ret;
        int *p = &ret;
        for (std::size_t i = 0; i < sizeof(int); i++)
        {
            *p = source[i];
            p++;
        }
        source.erase(source.cbegin(), source.cbegin() + sizeof(int));
        return ret;
    }

    auto PeekSize(std::deque<char> &source) -> std::size_t
    {
        std::size_t ret;
        std::size_t *p = &ret;
        for (std::size_t i = 0; i < sizeof(std::size_t); i++)
        {
            *p = source[i];
            p++;
        }
        source.erase(source.cbegin(), source.cbegin() + sizeof(std::size_t));
        return ret;
    }

    auto PeekString(std::deque<char> &source, std::size_t size) -> std::string
    {
        std::string ret;
        ret.append(source.cbegin(), source.cbegin() + size);
        source.erase(source.cbegin(), source.cbegin() + size);
        return ret;
    }

} // namespace rds

#endif