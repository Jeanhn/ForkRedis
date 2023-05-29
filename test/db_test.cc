#include <database/db.h>
#include <gtest/gtest.h>
#include <objects/list.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <objects/hash.h>
#include "util4test.h"
#ifndef NDEBUG

void CheckWhat(const std::string &what)
{
    std::cout << "check: " << what << std::endl;
}

TEST(KeyValue, kv)
{
    using namespace rds;
    Str key;
    Set value;
    key.Set(std::to_string(10085));
    for (int i = 0; i < 1000; i++)
    {
        std::string s;
        s.append(std::to_string(i));
        for (int k = 0; k < i / 100 + 1; k++)
        {
            for (int j = 0; j < 26; j++)
            {
                s.push_back('a' + j);
            }
        }
        Str str(std::move(s));

        value.Add(str);
    }

    auto v = std::make_unique<Set>(std::move(value));
    KeyValue kv(key, std::move(v));
    kv.MakeExpireAt(100);

    std::deque<char> src;
    std::string code = kv.Encode();
    std::copy(code.cbegin(), code.cend(), std::back_inserter(src));

    KeyValue kv2;
    kv2.Decode(&src);

    ASSERT_EQ(kv2.GetExpire().value(), 100);

    ASSERT_EQ(kv2.GetKey(), kv.GetKey());

    auto _v1 = kv.GetValue().lock().get();
    auto _v2 = kv.GetValue().lock().get();

    ASSERT_EQ(_v1->GetObjectType(), _v2->GetObjectType());

    auto sv = reinterpret_cast<Set *>(_v1);
    auto sv2 = reinterpret_cast<Set *>(_v2);

    auto m = sv->Members();
    auto m2 = sv2->Members();

    std::sort(m.begin(), m.end());
    std::sort(m2.begin(), m2.end());

    ASSERT_EQ(m, m2);

    std::vector<KeyValue> kvec;
    for (int i = 0; i < 1000; i++)
    {
        Str s(std::to_string(i));
        auto sp = std::make_unique<Str>(s);
        kvec.emplace_back(s, std::move(sp));
    }
    // decltype(kvec) kvec2;
}
TEST(Database, Db)
{
    using namespace rds;

    Log("checking db en-de-code");
    auto dbfile = db.Save();
    std::deque<char> cache;
    std::copy(dbfile.cbegin(), dbfile.cend(), std::back_inserter(cache));

    Db db2;
    db2.Load(&cache);

    DbEqual(&db, &db2);

    Log("checking db fork");
    Db cli_db;
    auto cli = std::make_shared<ClientInfo>();
    cli->database_ = &cli_db;
    auto raw_cmds = db2.Fork();
    for (auto &cmd : raw_cmds)
    {
        auto req = json11::Json(RawCommandToRequest(cmd)).dump();
        std::copy(req.cbegin(), req.cend(), std::back_inserter(cli->recv_buffer_));
        auto requests = cli->ExportMessages();
        for (auto &request : requests)
        {
            auto cmd = RequestToCommandExec(cli, &request);
            ASSERT_TRUE(cmd);
            cmd->Exec();
        }
    }
    DbEqual(&cli_db, &db2);
}

#endif