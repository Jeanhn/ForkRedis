#include <lzfse.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <json11.hpp>
#include <gtest/gtest.h>
#include <HTTPRequest.hpp>

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
    tempForJson tp;
    tp.val = "yeah";
    Json js(tp);
    auto arr = js.array_items();

    Json js2;
    std::string err;
    js2 = Json::parse(js.dump(), err, json11::STANDARD);

    std::cout << js2.array_items().at(2).string_value() << std::endl;
}

TEST(Third, HTTP)
{
    json11::Json js = json11::Json::array{"GET", "KEY"};
    http::Request request{"127.0.0.1:8080"};
    std::string body = js.dump();
    request.send("POST", body, {"Content-Type", "application/json"});
}