#include <database/db.h>
#include <objects/list.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <objects/hash.h>
#include <cstring>

namespace rds
{

    auto KeyValue::KeyEncode() -> std::string
    {
        std::string ret;

        if (expire_time_stamp_.has_value()) // the entry has expire
        {
            char c = EncodingTypeToChar(EncodingType::EXPIRE);
            ret.push_back(c);
            std::string expr_ms = BitsToString(expire_time_stamp_.value());
            ret.append(expr_ms);
        }
        ret.append(key_.EncodeValue());
        return ret;
    }

    auto KeyValue::Encode() -> std::string
    {
        std::string ret;
        ret.append(KeyEncode());
        ret.append(value_->EncodeValue());
        return ret;
    }

    auto KeyValue::GetValue() -> Object *
    {
        return value_.get();
    }

    auto KeyValue::GetKey() -> Str
    {
        return key_;
    }

    auto ExpireDecode(std::deque<char> &source) -> std::optional<std::size_t>
    {
        EncodingType etyp = CharToEncodingType(source.front());
        if (etyp != EncodingType::EXPIRE)
        {
            return {};
        };
        source.pop_front();
        std::size_t ret = PeekSize(source);
        std::optional<std::size_t> r(ret);
        return r;
    }
}

/*










 */
namespace rds
{
    template <typename T,
              typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
    void Db::NewStr(const Str &key, T &&value)
    {
        auto str = std::make_unique<Str>(std::forward<T>(value));
        KeyValue kv{key, std::move(str)};
        key_value_map_.insert({key, std::move(kv)});
    }

    void Db::NewList(const Str &key)
    {
        auto l = std::make_unique<List>();
        KeyValue kv{key, std::move(l)};
        key_value_map_.insert({key, std::move(kv)});
    }

    void Db::NewSet(const Str &key)
    {
        auto s = std::make_unique<Set>();
        KeyValue kv{key, std::move(s)};
        key_value_map_.insert({key, std::move(kv)});
    }

    void Db::NewZSet(const Str &key)
    {
        auto zs = std::make_unique<ZSet>();
        KeyValue kv{key, std::move(zs)};
        key_value_map_.insert({key, std::move(kv)});
    }

    void Db::NewHash(const Str &key)
    {
        auto h = std::make_unique<Hash>();
        KeyValue kv{key, std::move(h)};
        key_value_map_.insert({key, std::move(kv)});
    }

    void Db::Del(const Str &key)
    {
        str_lru_.Del(key);
        key_value_map_.erase(key);
    }

    auto Db::Get(const Str &key) -> Object *
    {
        auto it = key_value_map_.find(key);
        if (it == key_value_map_.end())
        {
            return nullptr;
        }
        return it->second.GetValue();
    }

    void Db::ExpireAtTime(const Str &key, std::size_t time_stamp)
    {
        auto it = key_value_map_.find(key);
        if (it == key_value_map_.end())
        {
            return;
        }
        it->second.MakeExpire(time_stamp);
        str_lru_.Set(key, time_stamp);
    }

    void Db::Expire(const Str &key, std::size_t time_period_s)
    {
        PExpire(key, time_period_s * 1000);
    }

    void Db::PExpire(const Str &key, std::size_t time_period_ms)
    {
        ExpireAtTime(key, UsTime() + time_period_ms * 1000);
    }
}
