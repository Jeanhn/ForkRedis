#ifndef __UTIL_H__
#define __UTIL_H__

#include <configure.h>
#include <cassert>
#include <utility>
#include <tuple>
#include <type_traits>
#include <memory>
#include <mutex>
#include <optional>
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
#include <iostream>
#define Panic()                         \
    std::cout << __FILE__ << std::endl; \
    std::abort();

void *operator new(size_t size);

void *operator new[](size_t size);

void *operator new(size_t size, std::nothrow_t &);

void *operator new[](size_t size, std::nothrow_t &);

void operator delete(void *p) noexcept;

void operator delete(void *p, size_t size) noexcept;

void operator delete(void *p, std::nothrow_t &);

void operator delete[](void *p) noexcept;

void operator delete[](void *p, size_t size) noexcept;

void operator delete[](void *p, std::nothrow_t &);

void *operator new(size_t size, std::align_val_t val);

void *operator new[](size_t size, std::align_val_t val);

void *operator new(size_t size, std::align_val_t val, std::nothrow_t &);

void *operator new[](size_t size, std::align_val_t val, std::nothrow_t &);

void operator delete(void *p, std::align_val_t) noexcept;

void operator delete[](void *p, std::align_val_t) noexcept;

void operator delete(void *p, size_t size, std::align_val_t val) noexcept;

void operator delete[](void *p, size_t size, std::align_val_t val) noexcept;

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

#define CLASS_DECLARE_uncopyable(name)             \
    name(const name &) = delete;                   \
    name(name &&) noexcept = default;              \
    auto operator=(const name &)->name & = delete; \
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

    auto UsTime(void) -> std::size_t;

    auto MsTime(void) -> std::size_t;

    template <typename BitType>
    inline auto BitsToString(BitType data) -> std::string
    {
        std::string ret;
        char buf[sizeof(BitType) + 2];
        memcpy(buf, &data, sizeof(BitType));
        ret.append(std::cbegin(buf), std::cbegin(buf) + sizeof(BitType));
        return ret;
    }

    auto PeekInt(std::deque<char> *source) -> int;

    auto PeekSize(std::deque<char> *source) -> std::size_t;

    auto PeekString(std::deque<char> *source, std::size_t size) -> std::string;

    auto Decompress(const std::string &data) -> std::string;

    auto Compress(const std::string &data) -> std::string;

    auto DefineCompress() -> bool;

    void EnCompress();

    void DisCompress();

    inline void Assert(bool expr, const std::string &info)
    {
        if (!expr)
        {
            throw std::runtime_error(info);
        }
    }

    template <typename T, typename... Ts>
    inline void Log(T arg, Ts... args)
    {
        std::cout << arg;
        if constexpr (sizeof...(args) != 0)
        {
            std::cout << ' ';
            Log(args...);
        }
        else
        {
            std::cout << std::endl;
        }
    }

    struct RedisConf
    {
        const char *ip_;
        short port_;
    };

} // namespace rds

#endif