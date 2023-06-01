#include <server/command.h>
#include <objects/list.h>
#include <objects/hash.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <server/server.h>
#include <condition_variable>
#include <server/loop.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace rds
{
    auto RawCommandToRequest(const std::string &raw) -> json11::Json::array
    {
        auto it = raw.cbegin();
        auto isTokenMember = [](char c)
        {
            return (c >= '0' && c <= '9') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z');
        };
        auto nextToken = [&raw, &it, &isTokenMember]() mutable -> std::string
        {
            if (it == raw.cend())
            {
                return {};
            }
            while (!isTokenMember(*it))
            {
                it++;
                if (it == raw.cend())
                {
                    return {};
                }
            }
            auto end = it;
            while (isTokenMember(*end))
            {
                end++;
                if (end == raw.end())
                {
                    break;
                }
            }
            std::string ret;
            std::copy(it, end, std::back_inserter(ret));
            it = end;
            return ret;
        };

        json11::Json::array ret;
        for (std::string token = nextToken(); !token.empty(); token = nextToken())
        {
            ret.push_back(std::move(token));
        }
        return ret;
    }

    auto JsonToBase(CommandBase *ret, const json11::Json::array &source) -> bool
    {
        if (source.size() < 2)
        {
            ret->valid_ = false;
            return false;
        }
        for (auto &e : source)
        {
            if (e.string_value().empty())
            {
                ret->valid_ = false;
                return false;
            }
        }
        ret->command_ = source[0].string_value();
        ret->obj_name_ = source[1].string_value();
        ret->valid_ = true;
        return true;
    }

    auto JsonToStrCommand(const json11::Json::array &source) -> StrCommand
    {
        StrCommand ret;
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        if (ret.command_ == "GET" || ret.command_ == "LEN")
        {
            return ret;
        }
        if (source.size() < 3)
        {
            ret.valid_ = false;
            return ret;
        }
        ret.value_ = source[2].string_value();
        return ret;
    }

    auto JsonToListCommand(const json11::Json::array &source) -> ListCommand
    {
        ListCommand ret;
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        if (ret.command_ == "LPOPF" ||
            ret.command_ == "LPOPB" ||
            ret.command_ == "LLEN")
        {
            return ret;
        }
        if (source.size() < 3)
        {
            ret.valid_ = false;
            return ret;
        }
        if (ret.command_ == "LREM" ||
            ret.command_ == "LSET" ||
            ret.command_ == "LTRIM")
        {
            if (source.size() < 4)
            {
                ret.valid_ = false;
                return ret;
            }
        }
        for (std::size_t i = 2; i < source.size(); i++)
        {
            ret.values_.push_back({source[i].string_value()});
        }
        return ret;
    }

    auto JsonToSetCommand(const json11::Json::array &source) -> SetCommand
    {
        SetCommand ret;
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        if (ret.command_ == "SCARD" ||
            ret.command_ == "SRANDMEMBER" ||
            ret.command_ == "SMEMBERS" ||
            ret.command_ == "SPOP")
        {
            return ret;
        }
        if (source.size() < 3)
        {
            ret.valid_ = false;
            return ret;
        }
        for (std::size_t i = 2; i < source.size(); i++)
        {
            ret.values_.push_back({source[i].string_value()});
        }
        return ret;
    }

    auto JsonToZSetCommand(const json11::Json::array &source) -> ZSetCommand
    {
        ZSetCommand ret;
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        if (ret.command_ == "ZCARD")
        {
            return ret;
        }
        if (source.size() < 3)
        {
            ret.valid_ = false;
            return ret;
        }
        if (source.size() % 2 != 0 &&
            ret.command_ != "ZSCORE" &&
            ret.command_ != "ZRANK")
        {
            ret.valid_ = false;
            return ret;
        }
        for (std::size_t i = 2; i < source.size(); i++)
        {
            ret.values_.push_back({source[i].string_value()});
        }
        return ret;
    }

    auto JsonToHashCommand(const json11::Json::array &source) -> HashCommand
    {
        HashCommand ret;
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        if (ret.command_ == "HLEN" || ret.command_ == "HGETALL")
        {
            return ret;
        }
        if (source.size() < 3)
        {
            ret.valid_ = false;
            return ret;
        }
        if (ret.command_ == "HSET" ||
            ret.command_ == "HINCRBY" ||
            ret.command_ == "HDECRBY")
        {
            if (source.size() % 2 != 0)
            {
                ret.valid_ = false;
                return ret;
            }
        }
        for (std::size_t i = 2; i < source.size(); i++)
        {
            ret.values_.push_back(source[i].string_value());
        }
        return ret;
    }

    auto JsonToDbCommand(const json11::Json::array &source) -> DbCommand
    {
        DbCommand ret;
        if (!source.empty())
        {
            if (source[0].string_value() == "REWRITE")
            {
                ret.valid_ = true;
                ret.command_ = "REWRITE";
                return ret;
            }
        }
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        if (ret.command_ == "DEL" || ret.command_ == "WHEN")
        {
            return ret;
        }
        if (source.size() < 3)
        {
            ret.valid_ = false;
            return ret;
        }
        ret.value_ = source[2].string_value();
        return ret;
    }

    auto JsonToCliCommand(const json11::Json::array &source) -> CliCommand
    {
        CliCommand ret;
        if (source.empty())
        {
            ret.valid_ = false;
            return ret;
        }
        ret.command_ = source[0].string_value();
        if (ret.command_ != "CREATE" && ret.command_ != "SHOW")
        {
            if (source.size() < 2)
            {
                ret.valid_ = false;
            }
            else
            {
                ret.obj_name_ = source[1].string_value();
                ret.valid_ = true;
                if (source.size() == 3)
                {
                    ret.pswd_ = source[2].string_value();
                }
            }
        }
        return ret;
    }

    auto JsonToServerCommand(const json11::Json::array &source) -> ServerCommand
    {
        ServerCommand ret;
        if (!source.empty())
        {
            if (source[0].string_value() == "ENDFORKING")
            {
                ret.command_ = "ENDFORKING";
                ret.valid_ = true;
                return ret;
            }
        }
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        std::string &info = ret.obj_name_;
        auto devide = std::find(info.cbegin(), info.cend(), ':');
        if (devide == info.cend())
        {
            ret.valid_ = false;
            return ret;
        }
        std::copy(info.cbegin(), devide, std::back_inserter(ret.ip_));
        std::string port;
        std::copy(devide + 1, info.cend(), std::back_inserter(port));
        ret.port_ = std::stoi(port);
        return ret;
    }

    /*





     */

    auto RequestToCommandExec(std::shared_ptr<ClientInfo> client, json11::Json::array *request) -> std::unique_ptr<CommandBase>
    {
        json11::Json::array &req = *request;
        auto isServerCommand = [](const std::string &cmd)
        {
            return (cmd == "FORK" ||
                    cmd == "ENDFORKING");
        };
        auto isCliCommand = [](const std::string &cmd)
        {
            return (cmd == "SELECT" ||
                    // cmd == "DROP" ||
                    cmd == "CREATE" ||
                    cmd == "SHOW");
        };
        auto isDbCommand = [](const std::string &cmd)
        {
            return (cmd == "DEL" ||
                    cmd == "EXPIRE" ||
                    cmd == "WHEN" ||
                    cmd == "REWRITE");
        };
        auto isStrCommand = [](const std::string &cmd)
        {
            return (cmd == "SET" ||
                    cmd == "GET" ||
                    cmd == "APPEND" ||
                    cmd == "LEN" ||
                    cmd == "INCRBY" ||
                    cmd == "DECRBY");
        };
        auto isListCommand = [](const std::string &cmd)
        {
            return (cmd == "LPUSHF" ||
                    cmd == "LPUSHB" ||
                    cmd == "LPOPF" ||
                    cmd == "LPOPB" ||
                    cmd == "LINDEX" ||
                    cmd == "LREM" ||
                    cmd == "LTRIM" ||
                    cmd == "LLEN" ||
                    cmd == "LSET");
        };
        auto isSetCommand = [](const std::string &cmd)
        {
            return (cmd == "SADD" ||
                    cmd == "SCARD" ||
                    cmd == "SISMEMBER" ||
                    cmd == "SMEMBERS" ||
                    cmd == "SRANDMEMBER" ||
                    cmd == "SPOP" ||
                    cmd == "SREM" ||
                    cmd == "SINTER" ||
                    cmd == "SDIFF");
        };
        auto isZSetCommand = [](const std::string &cmd)
        {
            return (cmd == "ZADD" ||
                    cmd == "ZCARD" ||
                    cmd == "ZCOUNT" ||
                    cmd == "ZLEXCOUNT" ||
                    cmd == "ZINCRBY" ||
                    cmd == "ZDECRBY" ||
                    cmd == "ZREM" ||
                    cmd == "ZRANGE" ||
                    cmd == "ZRANGEBYSCORE" ||
                    cmd == "ZRANGEBYLEX" ||
                    cmd == "ZRANK" ||
                    cmd == "ZSCORE");
        };
        auto isHashCommand = [](const std::string &cmd)
        {
            return (cmd == "HGET" ||
                    cmd == "HSET" ||
                    cmd == "HEXIST" ||
                    cmd == "HDEL" ||
                    cmd == "HLEN" ||
                    cmd == "HGETALL" ||
                    cmd == "HINCRBY" ||
                    cmd == "HDECRBY");
        };
        if (req.empty())
        {
            return nullptr;
        }
        auto cmd = req[0].string_value();
        for (auto &c : cmd)
        {
            c = std::toupper(c);
        }
        req[0] = {cmd};
        std::unique_ptr<CommandBase> ret;
        if (isServerCommand(cmd))
        {
            ret = std::make_unique<ServerCommand>(JsonToServerCommand(req));
        }
        if (isCliCommand(cmd))
        {
            ret = std::make_unique<CliCommand>(JsonToCliCommand(req));
        }
        if (isDbCommand(cmd))
        {
            ret = std::make_unique<DbCommand>(JsonToDbCommand(req));
        }
        if (isStrCommand(cmd))
        {
            ret = std::make_unique<StrCommand>(JsonToStrCommand(req));
        }
        if (isListCommand(cmd))
        {
            ret = std::make_unique<ListCommand>(JsonToListCommand(req));
        }
        if (isHashCommand(cmd))
        {
            ret = std::make_unique<HashCommand>(JsonToHashCommand(req));
        }
        if (isSetCommand(cmd))
        {
            ret = std::make_unique<SetCommand>(JsonToSetCommand(req));
        }
        if (isZSetCommand(cmd))
        {
            ret = std::make_unique<ZSetCommand>(JsonToZSetCommand(req));
        }
        if (ret)
        {
            ret->cli_ = client;
        }
        return ret;
    }
    /*



     */

    auto StrCommand::Exec() -> std::optional<json11::Json::array>
    {
        if (!valid_)
        {
            return {{" "}};
        }

        auto client = cli_.lock();
        if (!client)
        {
            return {};
        }

        obj_ = client->GetDB()->Get({obj_name_}).lock();
        if (obj_ == nullptr)
        {
            if (command_ != "SET")
            {
                return {{" "}};
            }
            obj_ = client->GetDB()->NewStr({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::STR)
        {
            return {{" "}};
        }

        auto str = reinterpret_cast<Str *>(obj_.get());

        if (command_ == "SET")
        {
            str->Set(value_.value());
        }
        else if (command_ == "GET")
        {
            auto ret = str->GetRaw();
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return {{"\"" + ret + "\""}};
        }
        else if (command_ == "INCRBY")
        {
            auto intval = RedisStrToInt(value_.value());
            if (!intval.has_value())
            {
                return {{" "}};
            }
            auto ret = str->IncrBy(intval.value());
            if (ret.empty())
            {
                return {{" "}};
            }
            return {{ret}};
        }
        else if (command_ == "DECRBY")
        {
            auto intval = RedisStrToInt(value_.value());
            if (!intval.has_value())
            {
                return {{" "}};
            }
            auto ret = str->DecrBy(intval.value());
            if (ret.empty())
            {
                return {{" "}};
            }
            return {{ret}};
        }
        else if (command_ == "APPEND")
        {
            auto size = str->Append(std::move(value_.value()));
            return {{std::to_string(size)}};
        }
        else if (command_ == "LEN")
        {
            return {{std::to_string(str->Len())}};
        }

        return {{"OK"}};
    }

    /*




     */

    auto ListCommand::Exec() -> std::optional<json11::Json::array>
    {
        if (!valid_)
        {
            return {{" "}};
        }

        auto client = cli_.lock();
        if (!client)
        {
            return {};
        }

        obj_ = client->GetDB()->Get({obj_name_}).lock();
        if (obj_ == nullptr)
        {
            if (command_ != "LPUSHF" && command_ != "LPUSHB")
            {
                return {{" "}};
            }
            obj_ = client->GetDB()->NewList({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::LIST)
        {
            return {{" "}};
        }

        auto l = reinterpret_cast<List *>(obj_.get());
        if (command_ == "LPUSHF")
        {
            for (auto &it : values_)
            {
                l->PushFront(std::move(it));
            }
            return {{std::to_string(l->Len())}};
        }
        else if (command_ == "LPUSHB")
        {
            for (auto &it : values_)
            {
                l->PushBack(std::move(it));
            }
            return {{std::to_string(l->Len())}};
        }
        else if (command_ == "LPOPF")
        {
            auto str = l->PopFront();
            if (str.Empty())
            {
                return {{"(nil)"}};
            }
            return {{'\"' + str.GetRaw() + '\"'}};
        }
        else if (command_ == "LPOPB")
        {
            auto str = l->PopBack();
            if (str.Empty())
            {
                return {{"(nil)"}};
            }
            return {{'\"' + str.GetRaw() + '\"'}};
        }
        else if (command_ == "LINDEX")
        {
            json11::Json::array ret;
            for (auto &value : values_)
            {
                auto intval = RedisStrToInt(value);
                if (!intval.has_value())
                {
                    continue;
                }
                auto str = l->Index(intval.value());
                if (!str.Empty())
                {
                    ret.push_back('\"' + str.GetRaw() + '\"');
                }
            }
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return ret;
        }
        else if (command_ == "LLEN")
        {
            return {{std::to_string(l->Len())}};
        }
        else if (command_ == "LREM")
        {
            auto intval = RedisStrToInt(values_[0]);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            std::size_t n = l->Rem(intval.value(), values_[1]);
            return {{std::to_string(n)}};
        }
        else if (command_ == "LTRIM")
        {
            auto intval1 = RedisStrToInt(values_[0]);
            auto intval2 = RedisStrToInt(values_[1]);
            if (!(intval1.has_value() && intval2.has_value()))
            {
                return {{" "}};
            }
            bool ret = l->Trim(intval1.value(), intval2.value());
            if (!ret)
            {
                return {{" "}};
            }
        }
        else if (command_ == "LSET")
        {
            auto intval = RedisStrToInt(values_[0]);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            bool ret = l->Set(intval.value(), values_[1]);
            if (!ret)
            {
                return {{" "}};
            }
        }

        return {{"OK"}};
    }

    /*










     */
    auto HashCommand::Exec() -> std::optional<json11::Json::array>
    {
        if (!valid_)
        {
            return {{" "}};
        }

        auto client = cli_.lock();
        if (!client)
        {
            return {};
        }

        obj_ = client->GetDB()->Get({obj_name_}).lock();
        if (obj_ == nullptr)
        {
            if (command_ != "HSET")
            {
                return {{" "}};
            }
            obj_ = client->GetDB()->NewHash({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::HASH)
        {
            return {{" "}};
        }

        auto tbl = reinterpret_cast<Hash *>(obj_.get());
        if (command_ == "HSET")
        {
            for (std::size_t i = 0; i < values_.size(); i += 2)
            {
                tbl->Set(values_[i], std::move(values_[i + 1]));
            }
        }
        else if (command_ == "HGET")
        {
            json11::Json::array ret;
            for (auto &key : values_)
            {
                auto s = tbl->Get(key).GetRaw();
                if (s.empty())
                {
                    s = "(nil)";
                }
                ret.push_back('\"' + s + '\"');
            }
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return ret;
        }
        else if (command_ == "HEXIST")
        {
            json11::Json::array ret;
            for (auto &key : values_)
            {
                if (tbl->Exist(key))
                {
                    ret.push_back("1");
                }
                else
                {
                    ret.push_back("0");
                }
            }
            return ret;
        }
        else if (command_ == "HDEL")
        {
            for (auto &key : values_)
            {
                tbl->Del(key);
            }
        }
        else if (command_ == "HLEN")
        {
            return {{std::to_string(tbl->Len())}};
        }
        else if (command_ == "HGETALL")
        {
            json11::Json::array ret;
            auto all = tbl->GetAll();
            for (auto &kv : all)
            {
                ret.push_back('\"' + kv.first.GetRaw() + '\"');
                ret.push_back('\"' + kv.second.GetRaw() + '\"');
            }
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return ret;
        }
        else if (command_ == "HINCRBY")
        {
            auto intval = RedisStrToInt(values_[1]);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            auto val = tbl->IncrBy(values_[0], intval.value());
            if (val.empty())
            {
                return {{" "}};
            }
            return {{val}};
        }
        else if (command_ == "HDECRBY")
        {
            auto intval = RedisStrToInt(values_[1]);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            auto val = tbl->DecrBy(values_[0], intval.value());
            if (val.empty())
            {
                return {{" "}};
            }
            return {{val}};
        }

        return {{"OK"}};
    }
    /*





     */
    auto SetCommand::Exec() -> std::optional<json11::Json::array>
    {
        if (!valid_)
        {
            return {{" "}};
        }
        auto client = cli_.lock();
        if (!client)
        {
            return {};
        }

        obj_ = client->GetDB()->Get({obj_name_}).lock();
        if (obj_ == nullptr)
        {
            if (command_ != "SADD")
            {
                return {{" "}};
            }
            obj_ = client->GetDB()->NewSet({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::SET)
        {
            return {{" "}};
        }

        auto st = reinterpret_cast<Set *>(obj_.get());
        if (command_ == "SADD")
        {
            int cnt = 0;
            for (auto &element : values_)
            {
                bool ok = st->Add(std::move(element));
                if (ok)
                {
                    cnt++;
                }
            }
            return {{std::to_string(cnt)}};
        }
        else if (command_ == "SCARD")
        {
            return {{std::to_string(st->Card())}};
        }
        else if (command_ == "SISMEMBER")
        {
            json11::Json::array ret;
            for (auto &element : values_)
            {
                if (st->IsMember(element))
                {
                    ret.push_back("1");
                }
                else
                {
                    ret.push_back("0");
                }
            }
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return ret;
        }
        else if (command_ == "SMEMBERS")
        {
            json11::Json::array ret;
            auto m = st->Members();
            for (auto &element : m)
            {
                ret.push_back('\"' + std::move(element.GetRaw()) + '\"');
            }
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return ret;
        }
        else if (command_ == "SRANDMEMBER")
        {
            json11::Json::array ret;
            auto v = st->RandMember().GetRaw();
            if (v.empty())
            {
                return {{"(nil)"}};
            }
            ret.push_back({'\"' + std::move(v) + '\"'});
            return ret;
        }
        else if (command_ == "SPOP")
        {
            json11::Json::array ret;
            auto v = st->Pop().GetRaw();
            if (v.empty())
            {
                return {{"(nil)"}};
            }
            ret.push_back({'\"' + std::move(v) + '\"'});
            return ret;
        }
        else if (command_ == "SREM")
        {
            int cnt = 0;
            for (auto &element : values_)
            {
                bool ok = st->Rem(element);
                if (ok)
                {
                    cnt++;
                }
            }
            return {{std::to_string(cnt)}};
        }
        else if (command_ == "SINTER")
        {
            auto another_set = client->GetDB()->Get(values_[0]).lock();
            if (another_set == nullptr)
            {
                return {{" "}};
            }
            if (another_set->GetObjectType() != ObjectType::SET)
            {
                return {{" "}};
            }
            auto a_st = reinterpret_cast<Set *>(another_set.get());
            auto inter = st->Inter(*a_st);
            json11::Json::array ret;
            for (auto &element : inter)
            {
                ret.push_back(std::move(element.GetRaw()));
            }
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return ret;
        }
        else if (command_ == "SDIFF")
        {
            auto another_set = client->GetDB()->Get(values_[0]).lock();
            if (another_set == nullptr)
            {
                return {{" "}};
            }
            if (another_set->GetObjectType() != ObjectType::SET)
            {
                return {{" "}};
            }
            auto a_st = reinterpret_cast<Set *>(another_set.get());
            auto inter = st->Diff(*a_st);
            json11::Json::array ret;
            for (auto &element : inter)
            {
                ret.push_back(std::move(element.GetRaw()));
            }
            if (ret.empty())
            {
                return {{"(nil)"}};
            }
            return ret;
        }

        return {{"OK"}};
    }
    /*




     */
    auto ZSetCommand::Exec() -> std::optional<json11::Json::array>
    {
        if (!valid_)
        {
            return {{" "}};
        }
        auto client = cli_.lock();
        if (!client)
        {
            return {};
        }

        obj_ = client->GetDB()->Get({obj_name_}).lock();
        if (obj_ == nullptr)
        {
            if (command_ != "ZADD")
            {
                return {{" "}};
            }
            obj_ = client->GetDB()->NewZSet({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::ZSET)
        {
            return {{" "}};
        }

        auto zst = reinterpret_cast<ZSet *>(obj_.get());
        if (command_ == "ZADD")
        {
            int cnt = 0;
            for (std::size_t i = 0; i < values_.size(); i += 2)
            {
                auto intval = RedisStrToInt(values_[i]);
                if (!intval.has_value())
                {
                    continue;
                }
                auto ok = zst->Add(intval.value(), std::move(values_[i + 1]));
                if (ok)
                {
                    cnt++;
                }
            }
            return {{std::to_string(cnt)}};
        }
        else if (command_ == "ZCARD")
        {
            std::size_t cnt = zst->Card();
            return {{std::to_string(cnt)}};
        }
        else if (command_ == "ZREM")
        {
            int cnt = 0;
            for (std::size_t i = 0; i < values_.size(); i += 2)
            {
                auto intval = RedisStrToInt(values_[i]);
                if (!intval.has_value())
                {
                    continue;
                }
                bool ok = zst->Rem(intval.value(), std::move(values_[i + 1]));
                if (ok)
                {
                    cnt++;
                }
            }
            return {{std::to_string(cnt)}};
        }
        else if (command_ == "ZCOUNT")
        {
            auto intval1 = RedisStrToInt(values_[0]);
            auto intval2 = RedisStrToInt(values_[1]);
            if (!(intval1.has_value() && intval2.has_value()))
            {
                return {{" "}};
            }
            std::size_t cnt = zst->Count(intval1.value(), intval2.value());
            return {{std::to_string(cnt)}};
        }
        else if (command_ == "ZLEXCOUNT")
        {
            std::size_t cnt = zst->LexCount(values_[0], values_[1]);
            return {{std::to_string(cnt)}};
        }
        else if (command_ == "ZINCRBY")
        {
            auto intval = RedisStrToInt(values_[0]);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            auto val = zst->IncrBy(intval.value(), values_[1]);
            if (val.empty())
            {
                return {{" "}};
            }
            return {{val}};
        }
        else if (command_ == "ZDECRBY")
        {
            auto intval = RedisStrToInt(values_[0]);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            auto val = zst->DecrBy(intval.has_value(), values_[1]);
            if (val.empty())
            {
                return {{" "}};
            }
            return {{val}};
        }
        else if (command_ == "ZRANGE")
        {
            auto intval1 = RedisStrToInt(values_[0]);
            auto intval2 = RedisStrToInt(values_[1]);
            if (!(intval1.has_value() && intval2.has_value()))
            {
                return {{" "}};
            }
            auto res = zst->Range(intval1.value(), intval2.value());
            if (res.empty())
            {
                return {{"(nil)"}};
            }
            json11::Json::array ret;
            for (auto kv : res)
            {
                ret.push_back(kv.first.GetRaw());
                ret.push_back(std::to_string(kv.second));
            }
            return ret;
        }
        else if (command_ == "ZRANGEBYSCORE")
        {
            auto intval1 = RedisStrToInt(values_[0]);
            auto intval2 = RedisStrToInt(values_[1]);
            if (!(intval1.has_value() && intval2.has_value()))
            {
                return {{" "}};
            }
            auto res = zst->RangeByScore(intval1.value(), intval2.value());
            if (res.empty())
            {
                return {{"(nil)"}};
            }
            json11::Json::array ret;
            for (auto kv : res)
            {
                ret.push_back(kv.first.GetRaw());
                ret.push_back(std::to_string(kv.second));
            }
            return ret;
        }
        else if (command_ == "ZRANGEBYLEX")
        {
            auto res = zst->RangeByLex(values_[0], values_[1]);
            if (res.empty())
            {
                return {{"(nil)"}};
            }
            json11::Json::array ret;
            for (auto kv : res)
            {
                ret.push_back(kv.first.GetRaw());
                ret.push_back(std::to_string(kv.second));
            }
            return ret;
        }
        else if (command_ == "ZRANK")
        {
            auto rank = zst->Rank(values_[0]);
            if (rank.empty())
            {
                return {{"(nil)"}};
            }
            return {{rank}};
        }
        else if (command_ == "ZSCORE")
        {
            auto score = zst->Score(values_[0]);
            if (score.empty())
            {
                return {{"(nil)"}};
            }
            return {{score}};
        }

        return {{"OK"}};
    }
    /*



     */
    auto DbCommand::Exec() -> std::optional<json11::Json::array>
    {
        if (!valid_)
        {
            return {{" "}};
        }

        auto client = cli_.lock();
        if (!client)
        {
            return {};
        }

        if (command_ == "DEL")
        {
            std::size_t n = client->GetDB()->Del(obj_name_);
            return {{std::to_string(n)}};
        }
        else if (command_ == "EXPIRE")
        {
            auto time_point_us = RedisStrToSize(value_.value());
            if (!time_point_us.has_value())
            {
                return {{" "}};
            }
            auto tmr = client->GetDB()->ExpireAt(obj_name_, time_point_us.value());
            if (tmr)
            {
                GetGlobalLoop().EncounterTimer(std::move(tmr));
            }
        }
        else if (command_ == "WHEN")
        {
            auto when = client->GetDB()->WhenExpire(obj_name_);
            return {{when}};
        }
        else if (command_ == "REWRITE" && GetGlobalLoop().conf_.enable_aof_)
        {
            auto rewrite_tmr = std::make_unique<DbRewriteTimer>();
            rewrite_tmr->expire_time_us_ = 0;
            rewrite_tmr->fm_ = GetGlobalLoop().GetAOFTimer().fm_;
            rewrite_tmr->cache_ = "AOF";
            rewrite_tmr->cache_.append(GetGlobalLoop().DatabaseForkAll());
            GetGlobalLoop().EncounterTimer(std::move(rewrite_tmr));
        }

        return {{"OK"}};
    }
    /*



     */
    auto CliCommand::Exec() -> std::optional<json11::Json::array>
    {
        if (!valid_)
        {
            return {{" "}};
        }
        if (command_ == "SELECT")
        {
            auto intval = RedisStrToInt(obj_name_);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            auto client = cli_.lock();
            if (!client)
            {
                return {};
            }
            if (pswd_.has_value() && pswd_.value() == GetPassword())
            {
                client->SetDB(GetGlobalLoop().SuGetDB(intval.value()));
            }
            else
            {
                client->SetDB(GetGlobalLoop().GetDB(intval.value()));
            }
            return {{"Select: " + std::to_string(client->GetDB()->Number())}};
        }
        else if (command_ == "SHOW")
        {
            auto client = cli_.lock();
            if (!client)
            {
                return {};
            }
            int c_db = client->GetDB()->Number();
            auto all = GetGlobalLoop().ShowDB();
            return {{std::to_string(c_db), all}};
        }
        else if (command_ == "DROP")
        {
            auto intval = RedisStrToInt(obj_name_);
            if (!intval.has_value())
            {
                return {{" "}};
            }
            GetGlobalLoop().DropDB(intval.value());
        }
        else if (command_ == "CREATE")
        {
            auto client = cli_.lock();
            if (!client)
            {
                return {};
            }
            int new_db_num = GetGlobalLoop().CreateDB();
            if (new_db_num == -1)
            {
                return {{" "}};
            }
            return {{std::to_string(new_db_num)}};
        }

        return {{"OK"}};
    }

    auto ServerCommand::Exec() -> std::optional<json11::Json::array>
    {
        auto client = cli_.lock();
        if (!client)
        {
            return {};
        }
        if (command_ == "FORK")
        {
            sockaddr_in sa;
            sa.sin_addr.s_addr = inet_addr(ip_.data());
            sa.sin_family = AF_INET;
            sa.sin_port = htons(port_);
            int another_server = socket(AF_INET, SOCK_STREAM, 0);
            if (another_server == -1)
            {
                return {{" "}};
            }
            int ret = connect(another_server, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
            if (ret == -1)
            {
                return {{"Connect faied: " + ip_ + ':' + std::to_string(port_)}};
            }
            extern void SetNonBlock(int fd);
            SetNonBlock(another_server);

            std::deque<std::string> reqs = client->GetDB()->Fork();
            reqs.push_back("ENDFORKING");

            Log("Try forking to:");
            std::cout << ip_ << ':' << port_ << '\n'
                      << std::endl;

            for (auto &req : reqs)
            {
                auto beg = req.data();
                auto end = beg + req.size();
                while (beg != end)
                {
                    int len = write(another_server, beg, std::distance(beg, end));
                    if (len == -1)
                    {
                        if (errno != EAGAIN)
                        {
                            return {{"Write Error"}};
                        }
                        else
                        {
                            continue;
                        }
                    }
                    beg += len;
                }
            }
            SetNonBlock(another_server);
            char buf[65535];
            int len = 65535;
            while (len != 0 && len != -1)
            {
                len = read(another_server, buf, 65535);
            }
            close(another_server);
        }
        else if (command_ == "ENDFORKING")
        {
            client->Logout();
            return {{" "}};
        }
        return {{"OK"}};
    }
};