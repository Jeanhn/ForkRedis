#ifndef __DB_H__
#define __DB_H__

#include <util.h>
#include <objects/object.h>
#include <objects/str.h>
#include <map>
#include <unordered_map>
#include <util/str_lru.h>

namespace rds
{
    class KeyValue
    {
    private:
        Str key_;
        std::unique_ptr<Object> value_;
        std::optional<std::size_t> expire_time_stamp_;

        auto KeyEncode() -> std::string;

    public:
        auto Encode() -> std::string;

        auto MakeExpire(std::size_t time_stamp)
        {
            expire_time_stamp_ = time_stamp;
        }

        void UndoExpire()
        {
            expire_time_stamp_.reset();
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
    private:
        std::unordered_map<Str, KeyValue, decltype(&StrHash)> key_value_map_{0xff, StrHash};
        StrLRU str_lru_;

        template <typename T,
                  typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
        void NewStr(const Str &, T &&);
        void NewList(const Str &);
        void NewSet(const Str &);
        void NewZSet(const Str &);
        void NewHash(const Str &);

    public:
        auto ExportLRU() -> StrLRU *;

        void Del(const Str &);
        auto Get(const Str &) -> Object *;

        void ExpireAtTime(const Str &, std::size_t);
        void Expire(const Str &, std::size_t);
        void PExpire(const Str &, std::size_t);
        CLASS_DEFAULT_DECLARE(Db);
    };

} // namespace rds

#endif