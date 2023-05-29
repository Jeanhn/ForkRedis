#ifndef __DB_H__
#define __DB_H__

#include <util.h>
#include <objects/object.h>
#include <objects/str.h>
#include <map>
#include <unordered_map>

namespace rds
{
    class KeyValue
    {
    private:
        mutable std::shared_mutex latch_;
        Str key_;
        std::shared_ptr<Object> value_;
        std::optional<std::size_t> expire_time_us_;

        auto InternalPrefixEncode() const -> std::string;

    public:
        auto Encode() const -> std::string;

        void Decode(std::deque<char> *);

        auto MakeExpireAt(std::size_t time_stamp)
        {
            expire_time_us_ = time_stamp;
        }

        void UndoExpire()
        {
            expire_time_us_.reset();
        }

        auto GetExpire() const -> std::optional<std::size_t>
        {
            return expire_time_us_;
        }

        auto IsExpire() const -> bool
        {
            if (!expire_time_us_.has_value())
            {
                return false;
            }
            return expire_time_us_.value() < UsTime();
        }

        auto GetValue() const -> std::weak_ptr<Object>;

        auto GetKey() const -> Str;

        auto Fork() -> std::string;

        KeyValue(const Str &, std::shared_ptr<Object>);

        KeyValue() = default;
        ~KeyValue() = default;

        KeyValue(const KeyValue &);
        KeyValue(KeyValue &&);
        KeyValue &operator=(KeyValue &&);
        KeyValue &operator=(const KeyValue &);
    };

    auto ExpireDecode(std::deque<char> &) -> std::optional<std::size_t>;

    class Timer;

    class Db
    {
#ifdef NDEBUG
    private:
#else
    public:
#endif
        mutable std::shared_mutex latch_;
        static int number;
        constexpr static char SELECT_DB_ = 's';
        int number_;
        std::unordered_map<Str, std::shared_ptr<KeyValue>, decltype(&StrHash)> key_value_map_{0xff, StrHash};

    public:
        auto NewStr(const Str &) -> std::shared_ptr<Object>;
        auto NewList(const Str &) -> std::shared_ptr<Object>;
        auto NewSet(const Str &) -> std::shared_ptr<Object>;
        auto NewZSet(const Str &) -> std::shared_ptr<Object>;
        auto NewHash(const Str &) -> std::shared_ptr<Object>;

        auto Del(const Str &) -> std::size_t;
        auto Get(const Str &) const -> std::weak_ptr<Object>;

        auto Expire(const Str &, std::size_t) -> std::unique_ptr<Timer>;

        auto WhenExpire(const Str &) -> std::string;

        auto Save() const -> std::string;

        void Load(std::deque<char> *);

        auto Size() const -> std::size_t;

        auto Number() const -> int;

        auto Fork() -> std::vector<std::string>;

        Db();
        ~Db() = default;
    };

} // namespace rds

#endif