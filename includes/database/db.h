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
        Str key_;
        std::unique_ptr<Object> value_;
        std::optional<std::size_t> expire_time_stamp_;

        auto PrefixEncode() -> std::string;

    public:
        auto Encode() -> std::string;

        void Decode(std::deque<char> *);

        auto MakeExpire(std::size_t time_stamp)
        {
            expire_time_stamp_ = time_stamp;
        }

        void UndoExpire()
        {
            expire_time_stamp_.reset();
        }

        auto GetExpire() -> std::optional<std::size_t>
        {
            return expire_time_stamp_;
        }

        auto IsExpire() -> bool
        {
            if (!expire_time_stamp_.has_value())
            {
                return false;
            }
            return expire_time_stamp_.value() < UsTime();
        }

        auto GetValue() -> Object *;

        auto GetKey() -> Str;

        KeyValue(const Str &, std::unique_ptr<Object>);

        CLASS_DEFAULT_DECLARE(KeyValue);
    };

    auto ExpireDecode(std::deque<char> &) -> std::optional<std::size_t>;

    class Db
    {
#ifdef NDEBUG
    private:
#else
    public:
#endif
        static int number;
        constexpr static char SELECT_DB_ = 's';
        int number_;
        std::unordered_map<Str, KeyValue, decltype(&StrHash)> key_value_map_{0xff, StrHash};

    public:
        void NewStr(const Str &);
        void NewList(const Str &);
        void NewSet(const Str &);
        void NewZSet(const Str &);
        void NewHash(const Str &);

        void Del(const Str &);
        auto Get(const Str &) -> Object *;

        void Expire(const Str &, std::size_t);

        auto Save() -> std::string;

        void Load(std::deque<char> *);

        auto Size() -> std::size_t;

        auto Number() const -> int;

        CLASS_DECLARE_without_constructor(Db);
        Db();
    };

} // namespace rds

#endif