#include <lzfse.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <json11.hpp>
#include <gtest/gtest.h>

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
    Json obj = Json::object{{"k", "v"}};
    std::string err;
    auto obj3 = Json::parse(obj.dump(), err);
    std::cout << obj.dump() << std::endl;
    std::cout << obj3.dump() << "\nerr:" << err << std::endl;

    Json arr = Json::array{"1", "2"};
    auto obj4 = Json::parse(arr.dump(), err);
    std::cout << obj4.dump() << "\nerr:" << err << std::endl;
}

TEST(Third, Snmalloc)
{
    int *p = new int;
    delete p;
}
