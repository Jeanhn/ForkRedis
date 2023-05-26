#include <database/db.h>
#include <objects/list.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <objects/hash.h>
#include <cstring>

namespace rds
{
    KeyValue::KeyValue(const Str &k, std::unique_ptr<Object> v) : key_(k), value_(std::move(v)) {}

    auto KeyValue::PrefixEncode() const -> std::string
    {
        std::string ret;

        if (expire_time_stamp_.has_value()) // the entry has expire
        {
            char c = ObjectTypeToChar(ObjectType::EXPIRE_ENTRY);
            ret.push_back(c);
            std::string expr_ms = BitsToString(expire_time_stamp_.value());
            ret.append(expr_ms);
        }
        char otyp = ObjectTypeToChar(value_->GetObjectType());
        ret.push_back(otyp);
        return ret;
    }

    auto KeyValue::Encode() const -> std::string
    {
        std::string ret;
        ret.append(PrefixEncode());
        ret.append(key_.EncodeValue());
        ret.append(value_->EncodeValue());
        return ret;
    }

    void KeyValue::Decode(std::deque<char> *source)
    {
        ObjectType otyp = CharToObjectType(source->front());
        source->pop_front();

        if (otyp == ObjectType::EXPIRE_ENTRY)
        {
            auto s = PeekSize(source);
            expire_time_stamp_ = s;
        }
        else
        {
            expire_time_stamp_.reset();
        }

        if (otyp == ObjectType::EXPIRE_ENTRY)
        {
            otyp = CharToObjectType(source->front());
            source->pop_front();
        }

        key_.DecodeValue(source);

        switch (otyp)
        {
        case ObjectType::STR:
            value_ = std::make_unique<Str>();
            value_->DecodeValue(source);
            break;
        case ObjectType::LIST:
            value_ = std::make_unique<List>();
            value_->DecodeValue(source);
            break;
        case ObjectType::HASH:
            value_ = std::make_unique<Hash>();
            value_->DecodeValue(source);
            break;
        case ObjectType::SET:
            value_ = std::make_unique<Set>();
            value_->DecodeValue(source);
            break;
        case ObjectType::ZSET:
            value_ = std::make_unique<ZSet>();
            value_->DecodeValue(source);
            break;
        default:
            assert(0);
            break;
        }
    }

    auto KeyValue::GetValue() const -> Object *
    {
        return value_.get();
    }

    auto KeyValue::GetKey() const -> Str
    {
        return key_;
    }

    auto ExpireDecode(std::deque<char> *source) -> std::optional<std::size_t>
    {
        ObjectType etyp = CharToObjectType(source->front());
        if (etyp != ObjectType::EXPIRE_ENTRY)
        {
            return {};
        };
        source->pop_front();
        std::size_t ret_expire_us = PeekSize(source);
        std::optional<std::size_t> r(ret_expire_us);
        return r;
    }
}

/*










 */
namespace rds
{
    int Db::number = 0;
    Db::Db() : number_(number++) {}

    auto Db::Number() const -> int
    {
        return number_;
    }

    void Db::NewStr(const Str &key)
    {
        auto str = std::make_unique<Str>();
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
        key_value_map_.erase(key);
    }

    auto Db::Get(const Str &key) const -> Object *
    {
        auto it = key_value_map_.find(key);
        if (it == key_value_map_.end())
        {
            return nullptr;
        }
        return it->second.GetValue();
    }

    void Db::Expire(const Str &key, std::size_t time_period_s)
    {
        auto it = key_value_map_.find(key);
        if (it == key_value_map_.end())
        {
            return;
        }
        it->second.MakeExpire(UsTime() + time_period_s * 1000'000);
    }

    auto Db::Save() const -> std::string
    {
        std::string ret;
        ret.push_back(SELECT_DB_);
        ret.append(BitsToString(number_));
        ret.append(BitsToString(key_value_map_.size()));
        for (auto &kv : key_value_map_)
        {
            std::string v = kv.second.Encode();
            ret.append(v);
        }
        return ret;
    }

    void Db::Load(std::deque<char> *source)
    {
        char s = source->front();
        source->pop_front();
        if (s != SELECT_DB_)
        {
            throw std::runtime_error("Err loading db");
        }
        number_ = PeekInt(source);
        std::size_t n = PeekSize(source);
        for (std::size_t i = 0; i < n; i++)
        {
            KeyValue kv;
            kv.Decode(source);
            key_value_map_.insert({kv.GetKey(), std::move(kv)});
        }
    }
}
