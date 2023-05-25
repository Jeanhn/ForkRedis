#include <lzfse.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <json11.hpp>
#include <gtest/gtest.h>
#include <server/command.h>
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

TEST(Third, Snmalloc)
{
    int *p = new int;
    delete p;
}
