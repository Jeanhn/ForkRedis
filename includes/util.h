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
#define CLASS_DECLARE_special_copy_move(name) \
    name() = default;                         \
    ~name() = default;                        \
    name(const name &);                       \
    name(name &&) noexcept;                   \
    auto operator=(const name &)->name &;     \
    auto operator=(name &&) noexcept -> name &;

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

    class SpinMutex
    {
    private:
        std::atomic_bool mu_;

    public:
        SpinMutex() : mu_(false) {}
        SpinMutex(const SpinMutex &) : SpinMutex() {}
        SpinMutex(SpinMutex &&) : SpinMutex() {}
        ~SpinMutex() = default;
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

    class Str;
    auto RedisStrToInt(const Str &value) -> std::optional<int>;

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
        std::string file_name_;
        std::string ip_;
        short port_;
        bool compress_;
        bool enable_aof_;
        struct
        {
            std::size_t every_n_sec_;
            std::size_t save_n_times_;
        } frequence_;
        std::size_t mem_size_mbytes_;
        int cpu_num_;
    };

    auto DefaultConf() -> RedisConf;

    auto LoadConf() -> std::optional<RedisConf>;

} // namespace rds

#endif