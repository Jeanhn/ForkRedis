#include <server/command.h>
#include <objects/list.h>
#include <objects/hash.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <server/server.h>

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
        ret->command_ = source[0].string_value();
        ret->obj_name_ = source[1].string_value();
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
        if (source.size() % 2 != 0 && (ret.command_ != "ZSCORE" ||
                                       ret.command_ != "ZRANK" ||
                                       ret.command_ != "ZREM"))
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
        if (ret.command_ == "HSET" || ret.command_ == "HINCRBY" || ret.command_ == "HDECRBY")
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
        if (!JsonToBase(&ret, source))
        {
            return ret;
        }
        if (ret.command_ == "DEL")
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
        JsonToBase(&ret, source);
        return ret;
    }
    /*





     */

    auto RequestToCommandExec(ClientInfo *client, const json11::Json::array &req) -> std::unique_ptr<CommandBase>
    {
        auto isCliCommand = [](const std::string &cmd)
        {
            return (cmd == "SELECT" || cmd == "DROP");
        };
        auto isDbCommand = [](const std::string &cmd)
        {
            return (cmd == "DEL" || cmd == "EXPIRE");
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
                    cmd == "LLEN");
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
                    cmd == "ZRANGEBYLEX");
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
        std::unique_ptr<CommandBase> ret;
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

    auto StrCommand::Exec() -> json11::Json::array
    {
        if (!valid_)
        {
            return {" "};
        }

        obj_ = cli_->GetDB()->Get({obj_name_});
        if (obj_ == nullptr)
        {
            if (command_ != "SET")
            {
                return {" "};
            }
            cli_->GetDB()->NewStr({obj_name_});
            obj_ = cli_->GetDB()->Get({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::STR)
        {
            return {" "};
        }

        auto str = reinterpret_cast<Str *>(obj_);

        if (command_ == "SET")
        {
            str->Set(value_.value());
        }
        else if (command_ == "GET")
        {
            auto ret = str->GetRaw();
            if (ret.empty())
            {
                return {"(nil)"};
            }
            return {"\"" + ret + "\""};
        }
        else if (command_ == "INCRBY")
        {
            auto ret = str->IncrBy(std::stoi(value_.value()));
            if (ret.empty())
            {
                return {" "};
            }
            return {ret};
        }
        else if (command_ == "DECRBY")
        {
            auto ret = str->DecrBy(std::stoi(value_.value()));
            if (ret.empty())
            {
                return {" "};
            }
            return {ret};
        }
        else if (command_ == "APPEND")
        {
            auto size = str->Append(std::move(value_.value()));
            return {std::to_string(size)};
        }
        else if (command_ == "LEN")
        {
            return {std::to_string(str->Len())};
        }

        return {"OK"};
    }

    /*




     */

    auto ListCommand::Exec() -> json11::Json::array
    {
        if (!valid_)
        {
            return {" "};
        }

        obj_ = cli_->GetDB()->Get({obj_name_});
        if (obj_ == nullptr)
        {
            if (command_ != "LPUSHF" && command_ != "LPUSHB")
            {
                return {" "};
            }
            cli_->GetDB()->NewList({obj_name_});
            obj_ = cli_->GetDB()->Get({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::LIST)
        {
            return {" "};
        }

        auto l = reinterpret_cast<List *>(obj_);
        if (command_ == "LPUSHF")
        {
            for (auto &it : values_)
            {
                l->PushFront(std::move(it));
            }
            return {std::to_string(l->Len())};
        }
        else if (command_ == "LPUSHB")
        {
            for (auto &it : values_)
            {
                l->PushBack(std::move(it));
            }
            return {std::to_string(l->Len())};
        }
        else if (command_ == "LPOPF")
        {
            auto str = l->PopFront();
            if (str.Empty())
            {
                return {"(nil)"};
            }
            return {'\"' + str.GetRaw() + '\"'};
        }
        else if (command_ == "LPOPB")
        {
            auto str = l->PopBack();
            if (str.Empty())
            {
                return {"(nil)"};
            }
            return {'\"' + str.GetRaw() + '\"'};
        }
        else if (command_ == "LINDEX")
        {
            json11::Json::array ret;
            for (auto &value : values_)
            {
                auto str = l->Index(std::stoi(value.GetRaw()));
                if (!str.Empty())
                {
                    ret.push_back('\"' + str.GetRaw() + '\"');
                }
            }
            if (ret.empty())
            {
                return {"(nil)"};
            }
            return ret;
        }
        else if (command_ == "LLEN")
        {
            return {std::to_string(l->Len())};
        }
        else if (command_ == "LREM")
        {
            if (values_.size() < 2)
            {
                return {" "};
            }
            std::size_t n = l->Rem(std::stoi(values_[0].GetRaw()), values_[1]);
            return {std::to_string(n)};
        }
        else if (command_ == "LTRIM")
        {
            if (values_.size() < 2)
            {
                return {" "};
            }
            bool ret = l->Trim(std::stoi(values_[0].GetRaw()), std::stoi(values_[1].GetRaw()));
            if (!ret)
            {
                return {" "};
            }
        }
        else if (command_ == "LSET")
        {
            if (values_.size() < 2)
            {
                return {" "};
            }
            bool ret = l->Set(std::stoi(values_[0].GetRaw()), values_[1]);
            if (!ret)
            {
                return {" "};
            }
        }

        return {"OK"};
    }

    /*










     */
    auto HashCommand::Exec() -> json11::Json::array
    {
        if (!valid_)
        {
            return {" "};
        }

        obj_ = cli_->GetDB()->Get({obj_name_});
        if (obj_ == nullptr)
        {
            if (command_ != "HSET")
            {
                return {" "};
            }
            cli_->GetDB()->NewHash({obj_name_});
            obj_ = cli_->GetDB()->Get({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::HASH)
        {
            return {" "};
        }

        auto tbl = reinterpret_cast<Hash *>(obj_);
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
                return {"(nil)"};
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
            return {std::to_string(tbl->Len())};
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
                return {"(nil)"};
            }
            return ret;
        }
        else if (command_ == "HINCRBY")
        {
            auto val = tbl->IncrBy(values_[0], std::stoul(values_[1].GetRaw()));
            if (val.empty())
            {
                return {" "};
            }
            return {val};
        }
        else if (command_ == "HDECRBY")
        {
            auto val = tbl->DecrBy(values_[0], std::stoul(values_[1].GetRaw()));
            if (val.empty())
            {
                return {" "};
            }
            return {val};
        }

        return {"OK"};
    }
    /*





     */
    auto SetCommand::Exec() -> json11::Json::array
    {
        if (!valid_)
        {
            return {" "};
        }

        obj_ = cli_->GetDB()->Get({obj_name_});
        if (obj_ == nullptr)
        {
            if (command_ != "SADD")
            {
                return {" "};
            }
            cli_->GetDB()->NewSet({obj_name_});
            obj_ = cli_->GetDB()->Get({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::SET)
        {
            return {" "};
        }

        auto st = reinterpret_cast<Set *>(obj_);
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
            return {std::to_string(cnt)};
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
                return {"(nil)"};
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
                return {"(nil)"};
            }
            return ret;
        }
        else if (command_ == "SRANDMEMBER")
        {
            json11::Json::array ret;
            auto v = st->RandMember().GetRaw();
            if (v.empty())
            {
                return {"(nil)"};
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
                return {"(nil)"};
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
            return {std::to_string(cnt)};
        }
        else if (command_ == "SINTER")
        {
            Object *another_set = cli_->GetDB()->Get(values_[0]);
            if (another_set->GetObjectType() != ObjectType::SET)
            {
                return {" "};
            }
            auto a_st = reinterpret_cast<Set *>(another_set);
            auto inter = st->Inter(*a_st);
            json11::Json::array ret;
            for (auto &element : inter)
            {
                ret.push_back(std::move(element.GetRaw()));
            }
            if (ret.empty())
            {
                return {"(nil)"};
            }
            return ret;
        }
        else if (command_ == "SDIFF")
        {
            Object *another_set = cli_->GetDB()->Get(values_[0]);
            if (another_set->GetObjectType() != ObjectType::SET)
            {
                return {" "};
            }
            auto a_st = reinterpret_cast<Set *>(another_set);
            auto inter = st->Diff(*a_st);
            json11::Json::array ret;
            for (auto &element : inter)
            {
                ret.push_back(std::move(element.GetRaw()));
            }
            if (ret.empty())
            {
                return {"(nil)"};
            }
            return ret;
        }

        return {"OK"};
    }
    /*




     */
    auto ZSetCommand::Exec() -> json11::Json::array
    {
        if (!valid_)
        {
            return {" "};
        }

        obj_ = cli_->GetDB()->Get({obj_name_});
        if (obj_ == nullptr)
        {
            if (command_ != "ZADD")
            {
                return {" "};
            }
            cli_->GetDB()->NewZSet({obj_name_});
            obj_ = cli_->GetDB()->Get({obj_name_});
        }
        if (obj_->GetObjectType() != ObjectType::ZSET)
        {
            return {" "};
        }

        auto zst = reinterpret_cast<ZSet *>(obj_);
        if (command_ == "ZADD")
        {
            int cnt;
            for (std::size_t i = 0; i < values_.size(); i += 2)
            {
                auto ok = zst->Add(std::stoi(values_[i].GetRaw()), std::move(values_[i + 1]));
                if (ok)
                {
                    cnt++;
                }
            }
            return {std::to_string(cnt)};
        }
        else if (command_ == "ZCARD")
        {
            std::size_t cnt = zst->Card();
            return {std::to_string(cnt)};
        }
        else if (command_ == "ZREM")
        {
            for (std::size_t i = 0; i < values_.size(); i += 2)
            {
                zst->Rem(std::stoi(values_[i].GetRaw()), std::move(values_[i + 1]));
            }
        }
        else if (command_ == "ZCOUNT")
        {
            std::size_t cnt = zst->Count(std::stoi(values_[0].GetRaw()), std::stoi(values_[1].GetRaw()));
            return {std::to_string(cnt)};
        }
        else if (command_ == "ZLEXCOUNT")
        {
            std::size_t cnt = zst->LexCount(values_[0], values_[1]);
            return {std::to_string(cnt)};
        }
        else if (command_ == "ZINCRBY")
        {
            auto val = zst->IncrBy(std::stoi(values_[0].GetRaw()), values_[1]);
            if (val.empty())
            {
                return {" "};
            }
            return {val};
        }
        else if (command_ == "ZDECRBY")
        {
            auto val = zst->DecrBy(std::stoi(values_[0].GetRaw()), values_[1]);
            if (val.empty())
            {
                return {" "};
            }
            return {val};
        }
        else if (command_ == "ZRANGE")
        {
            auto res = zst->Range(std::stoi(values_[0].GetRaw()), std::stoi(values_[1].GetRaw()));
            if (res.empty())
            {
                return {"(nil)"};
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
            auto res = zst->RangeByScore(std::stoi(values_[0].GetRaw()), std::stoi(values_[1].GetRaw()));
            if (res.empty())
            {
                return {"(nil)"};
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
                return {"(nil)"};
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
                return {"(nil)"};
            }
            return {rank};
        }
        else if (command_ == "ZSCORE")
        {
            auto score = zst->Score(values_[0]);
            if (score.empty())
            {
                return {"(nil)"};
            }
            return {score};
        }

        return {"OK"};
    }
    /*



     */
    auto DbCommand::Exec() -> json11::Json::array
    {
        assert(0);
        if (!valid_)
        {
            return {""};
        }

        if (cli_->GetDB() == nullptr)
        {
            return {""};
        }

        if (command_ == "DEL")
        {
            cli_->GetDB()->Del({obj_name_});
        }
        else if (command_ == "EXPIRE")
        {
            cli_->GetDB()->Expire({obj_name_}, std::stoul(value_.value()));
        }

        return {"OK"};
    }
    /*



     */
    auto CliCommand::Exec() -> json11::Json::array
    {
        assert(0);
        if (!valid_)
        {
            return {""};
        }
        if (command_ == "SELECT")
        {
            cli_->ShiftDB(std::stoi(value_.value()));
        }

        return {"OK"};
    }
};