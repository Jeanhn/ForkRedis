#include <lzfse.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <json11.hpp>
#include <gtest/gtest.h>
#include <server/command.h>
#include <util.h>
#include <fstream>
using std::cout;
using std::endl;

struct tempForJson
{
    char id;
    int key;
    std::string val;
    json11::Json to_json() const
    {
        return json11::Json::array{id, key, val};
    }
};

TEST(Third, Json11)
{
    using namespace json11;
    using namespace rds;
    Json cmd = RawCommandToRequest("SET STR GOOD");
    cout << cmd.dump() << endl;
}

TEST(Third, stoi)
{
    for (int i = 1; i < 19999; i++)
    {
        std::string s("-");
        s.append(std::to_string(i));
        auto val = rds::RedisStrToInt(s);
        ASSERT_EQ(val, -i);
    }
}

TEST(Third, conf)
{
    auto conf = rds::DefaultConf();
    json11::Json::object obj;
    obj["dbfile"] = "dump.db";
    obj["ip"] = "127.0.0.1";
    obj["port"] = 8080;
    obj["compress"] = true;
    obj["aof"] = false;
    obj["sec"] = 1;
    obj["time"] = 1;
    obj["memsiz_mb"] = 4096;
    obj["cpu"] = 2;
    obj["password"] = "yeah";
    json11::Json o(obj);
    std::fstream f;
    f.open("redis-conf.json");
    f << o.dump();
}
