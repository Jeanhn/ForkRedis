#include <lzfse.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <json11.hpp>
#include <gtest/gtest.h>
#include <server/command.h>
#include <util.h>
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
