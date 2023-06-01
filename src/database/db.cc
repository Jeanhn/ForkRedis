#include <database/db.h>
#include <objects/list.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <objects/hash.h>
#include <cstring>
#include <util.h>
#include <server/timer.h>
#include <server/loop.h>
#include <stack>

namespace rds
{
    KeyValue::KeyValue(const Str &k, std::shared_ptr<Object> v) : key_(k), value_(std::move(v)) {}

    KeyValue::KeyValue(const KeyValue &lhs)
    {
        ReadGuard rg(lhs.latch_);
        key_ = lhs.key_;
        value_ = lhs.value_;
    }
    KeyValue::KeyValue(KeyValue &&rhs)
    {
        ReadGuard rg(rhs.latch_);
        key_ = std::move(rhs.key_);
        value_ = std::move(rhs.value_);
    }
    KeyValue &KeyValue::operator=(KeyValue &&rhs)
    {
        ReadGuard rg(rhs.latch_);
        key_ = std::move(rhs.key_);
        value_ = std::move(rhs.value_);
        return *this;
    }
    KeyValue &KeyValue::operator=(const KeyValue &lhs)
    {
        ReadGuard rg(lhs.latch_);
        key_ = lhs.key_;
        value_ = lhs.value_;
        return *this;
    }

    auto KeyValue::InternalPrefixEncode() const -> std::string
    {
        std::string ret;
        if (expire_time_us_.has_value()) // the entry has expire
        {
            char c = ObjectTypeToChar(ObjectType::EXPIRE_ENTRY);
            ret.push_back(c);
            std::string expr_us = BitsToString(expire_time_us_.value());
            ret.append(expr_us);
        }
        char otyp = ObjectTypeToChar(value_->GetObjectType());
        ret.push_back(otyp);
        return ret;
    }

    auto KeyValue::Encode() const -> std::string
    {
        std::string ret;
        ReadGuard rg(latch_);
        ret.append(InternalPrefixEncode());
        ret.append(key_.EncodeValue());
        ret.append(value_->EncodeValue());
        return ret;
    }

    void KeyValue::Decode(std::deque<char> *source)
    {
        ObjectType otyp = CharToObjectType(source->front());
        source->pop_front();
        WriteGuard wg(latch_);

        if (otyp == ObjectType::EXPIRE_ENTRY)
        {
            auto s = PeekSize(source);
            expire_time_us_ = s;
        }
        else
        {
            expire_time_us_.reset();
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

    auto KeyValue::GetValue() const -> std::weak_ptr<Object>
    {
        ReadGuard rg(latch_);
        return value_;
    }

    auto KeyValue::GetKey() const -> Str
    {
        ReadGuard rg(latch_);
        return key_;
    }

    auto KeyValue::Fork() -> std::string
    {
        ReadGuard rg(latch_);
        return value_->Fork(key_.GetRaw());
    }
}

/*










 */
namespace rds
{
    static std::mutex smtx;
    static std::unordered_map<int, int> num_hash;
    static int number_source = 0;
    Db::Db()
    {
        std::lock_guard lg(smtx);
        while (num_hash[number_source])
        {
            number_source++;
        }
        num_hash[number_source] = 1;
        number_ = number_source;
    }

    Db::Db(int db_number)
    {
        std::lock_guard lg(smtx);
        while (num_hash[db_number])
        {
            db_number++;
        }
        num_hash[db_number] = 1;
        number_ = db_number;
    }

    Db::~Db()
    {
        std::lock_guard lg(smtx);
        num_hash[number_source] = 0;
    }

    auto Db::Number() const -> int
    {
        int no;
        {
            ReadGuard rg(latch_);
            no = number_;
        }
        return no;
    }

    auto Db::NewStr(const Str &key) -> std::shared_ptr<Object>
    {
        auto str = std::make_shared<Str>();
        auto kv = std::make_shared<KeyValue>(key, str);
        WriteGuard wg(latch_);
        key_value_map_.insert({key, kv});
        return str;
    }

    auto Db::NewList(const Str &key) -> std::shared_ptr<Object>
    {
        auto l = std::make_shared<List>();
        auto kv = std::make_shared<KeyValue>(key, l);
        WriteGuard wg(latch_);
        key_value_map_.insert({key, std::move(kv)});
        return l;
    }

    auto Db::NewSet(const Str &key) -> std::shared_ptr<Object>
    {
        auto st = std::make_shared<Set>();
        auto kv = std::make_shared<KeyValue>(key, st);
        WriteGuard wg(latch_);
        key_value_map_.insert({key, std::move(kv)});
        return st;
    }

    auto Db::NewZSet(const Str &key) -> std::shared_ptr<Object>
    {
        auto zst = std::make_shared<ZSet>();
        auto kv = std::make_shared<KeyValue>(key, zst);
        WriteGuard wg(latch_);
        key_value_map_.insert({key, std::move(kv)});
        return zst;
    }

    auto Db::NewHash(const Str &key) -> std::shared_ptr<Object>
    {
        auto hs = std::make_shared<Hash>();
        auto kv = std::make_shared<KeyValue>(key, hs);
        WriteGuard wg(latch_);
        key_value_map_.insert({key, std::move(kv)});
        return hs;
    }

    auto Db::Del(const Str &key) -> std::size_t
    {
        WriteGuard wg(latch_);
        return key_value_map_.erase(key);
    }

    auto Db::Get(const Str &key) const -> std::weak_ptr<Object>
    {
        ReadGuard rg(latch_);
        auto it = key_value_map_.find(key);
        if (it == key_value_map_.end())
        {
            return {};
        }
        return it->second->GetValue();
    }

    auto Db::ExpireAt(const Str &key, std::size_t time_point_us) -> std::unique_ptr<Timer>
    {
        ReadGuard wg(latch_);
        auto it = key_value_map_.find(key);
        if (it == key_value_map_.end())
        {
            return {};
        }
        it->second->MakeExpireAt(time_point_us);
        auto ret = std::make_unique<DbExpireTimer>();
        ret->database_ = this;
        ret->expire_time_us_ = time_point_us;
        ret->obj_name_ = key.GetRaw();
        return ret;
    }

    auto Db::WhenExpire(const Str &key) -> std::string
    {
        ReadGuard wg(latch_);
        auto it = key_value_map_.find(key);
        if (it == key_value_map_.end())
        {
            return "(nil)";
        }
        auto time_point = it->second->GetExpire();
        if (time_point.has_value())
        {
            return std::to_string((time_point.value() - UsTime()) / 990'000);
        }
        return "never";
    }

    auto Db::Save() const -> std::string
    {
        ReadGuard rg(latch_);
        if (key_value_map_.empty())
        {
            return {};
        }
        std::string ret;
        ret.push_back(SELECT_DB_);
        ret.append(BitsToString(number_));
        ret.append(BitsToString(key_value_map_.size()));
        for (auto &kv : key_value_map_)
        {
            std::string v = kv.second->Encode();
            ret.append(v);
        }
        return ret;
    }

    void Db::Load(std::deque<char> *source)
    {
        WriteGuard wg(latch_);
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
            auto kv = std::make_shared<KeyValue>();
            kv->Decode(source);
            auto expire_time_us = kv->GetExpire();
            if (expire_time_us.has_value())
            {
                auto exp_tmr = std::make_unique<DbExpireTimer>();
                exp_tmr->database_ = this;
                exp_tmr->obj_name_ = kv->GetKey().GetRaw();
                exp_tmr->expire_time_us_ = expire_time_us.value();
                GetGlobalLoop().EncounterTimer(std::move(exp_tmr));
            }
            key_value_map_.insert({kv->GetKey(), std::move(kv)});
        }
    }

    auto Db::Fork() -> std::deque<std::string>
    {
        std::deque<std::string> ret;
        std::string sel_cmd = "SELECT " + std::to_string(number_) + " " + GetPassword();
        json11::Json temp = RawCommandToRequest(sel_cmd);
        ret.push_back(temp.dump());
        ReadGuard rg(latch_);
        for (auto &kv : key_value_map_)
        {
            auto raw_cmd = kv.second->Fork();
            json11::Json cmd_temp = RawCommandToRequest(raw_cmd);
            ret.push_back(cmd_temp.dump());
        }
        return ret;
    }

    void Db::AppendAOF(const json11::Json::array &request)
    {
        auto req_dump = json11::Json(request).dump();
        std::lock_guard lg(aof_mtx_);
        std::copy(req_dump.cbegin(), req_dump.cend(), std::back_inserter(aof_cache_));
    }

    auto Db::ExportAOF() -> std::string
    {
        std::string select_cmd("SELECT " + std::to_string(number_) + " " + GetPassword());
        auto sel_req = RawCommandToRequest(select_cmd);
        std::string ret = json11::Json(sel_req).dump();
        std::lock_guard lg(aof_mtx_);
        if (aof_cache_.empty())
        {
            return {};
        }
        ret.reserve(aof_cache_.size());
        std::copy(aof_cache_.cbegin(), aof_cache_.cend(), std::back_inserter(ret));
        aof_cache_.clear();
        return ret;
    }

    void LoadAOF(std::deque<char> *source)
    {
    }
}
