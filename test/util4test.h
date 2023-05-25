#ifndef __UTIL4_TEST_H__
#define __UTIL4_TEST_H__

#include <gtest/gtest.h>
#include <database/disk.h>
#include <database/db.h>
#include <database/rdb.h>
#include <objects/list.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <objects/hash.h>
#include <iostream>

#include <server/server.h>
#include <server/command.h>
#include <server/timer.h>

#ifndef NDEBUG

template <typename T, typename... Ts>
void print(T Arg, Ts... Args)
{
    std::cout << Arg;
    if constexpr (sizeof...(Args) == 0)
    {
        std::cout << std::endl;
    }
    else
    {
        std::cout << ' ';
        print(Args...);
    }
}

auto RandStrArr(int num) -> std::vector<std::string>
{
    if (num < 0)
    {
        num = -num;
    }
    std::vector<std::string> raw_str_src;
    for (int i = 0; i < num; i++)
    {
        std::string s;
        s.append(std::to_string(i));
        for (int j = 20; j < 100; j++)
        {
            s.push_back('a' + j % 26);
        }
        raw_str_src.push_back(s);
    }
    return raw_str_src;
}

auto RandIntArr(int num) -> std::vector<std::string>
{
    if (num < 0)
    {
        num = -num;
    }
    std::vector<std::string> int_str_src;
    for (int i = 0xffff; i < 0xffff + num; i++)
    {
        int_str_src.push_back(std::to_string(i));
    }
    return int_str_src;
}

std::vector<std::string> key_suit, value_suit;
std::vector<std::pair<std::string, rds::Str>> str_suit;
std::vector<std::pair<std::string, rds::List>> list_suit;
std::vector<std::pair<std::string, rds::Set>> set_suit;
std::vector<std::pair<std::string, rds::ZSet>> zset_suit;
std::vector<std::pair<std::string, rds::Hash>> hash_suit;

rds::Db db;
rds::FileManager fm;
rds::Rdb rdb;

class Util4test
{
private:
    /* data */
    void InitSource()
    {
        key_suit = RandStrArr(100);
        auto temp = RandIntArr(100);
        std::copy(temp.cbegin(), temp.cend(), std::back_inserter(key_suit));
        std::copy(key_suit.rbegin(), key_suit.rend(), std::back_inserter(value_suit));
    }
    void InitStr()
    {
        for (std::size_t i = 0; i < key_suit.size(); i++)
        {
            str_suit.push_back({key_suit[i], value_suit[i]});
        }
    }
    void InitList()
    {
        for (auto &key : key_suit)
        {
            rds::List l;
            for (auto &v : value_suit)
            {
                l.PushBack(v);
            }
            list_suit.push_back({"List" + key, std::move(l)});
        }
    }
    void InitSet()
    {
        for (auto &key : key_suit)
        {
            rds::Set s;
            for (auto &v : value_suit)
            {
                s.Add(v);
            }
            set_suit.push_back({"Set" + key, std::move(s)});
        }
    }
    void InitZSet()
    {
        for (auto &key : key_suit)
        {
            rds::ZSet z;
            int i = 0;
            for (auto &v : value_suit)
            {
                z.Add(i, v);
                i++;
            }
            zset_suit.push_back({"ZSet" + key, std::move(z)});
        }
    }
    void InitHash()
    {
        for (auto &key : key_suit)
        {
            rds::Hash h;
            for (auto &v : value_suit)
            {
                h.Set(v, v);
            }
            hash_suit.push_back({"Hash" + key, std::move(h)});
        }
    }

    void InitDb()
    {
        for (auto cont : list_suit)
        {
            db.NewList(cont.first);
            auto obj = db.Get(cont.first);
            *(reinterpret_cast<rds::List *>(obj)) = cont.second;
        }
        for (auto cont : set_suit)
        {
            db.NewSet(cont.first);
            auto obj = db.Get(cont.first);
            *(reinterpret_cast<rds::Set *>(obj)) = cont.second;
            db.Expire(cont.first, 3);
        }
        for (auto cont : str_suit)
        {
            db.NewStr(cont.first);
            auto obj = db.Get(cont.first);
            *(reinterpret_cast<rds::Str *>(obj)) = cont.second;
            db.Expire(cont.first, 2);
        }
        for (auto cont : hash_suit)
        {
            db.NewHash(cont.first);
            auto obj = db.Get(cont.first);
            *(reinterpret_cast<rds::Hash *>(obj)) = cont.second;
        }
        for (auto cont : zset_suit)
        {
            db.NewZSet(cont.first);
            auto obj = db.Get(cont.first);
            *(reinterpret_cast<rds::ZSet *>(obj)) = cont.second;
            db.Expire(cont.first, 1);
        }
    }

public:
    Util4test(/* args */)
    {
        InitSource();
        InitStr();
        InitList();
        InitSet();
        InitZSet();
        InitHash();
        InitDb();
        std::cout << "Init complete" << std::endl;
    }
    ~Util4test() = default;
};

static Util4test mod;

auto DbEqual(rds::Db *d1, rds::Db *d2) -> void
{
    for (auto &kv : d1->key_value_map_)
    {
        auto pos = d2->key_value_map_.find(kv.first);
        assert(pos != d2->key_value_map_.end());
    }
    assert(d1->key_value_map_.size() == d2->key_value_map_.size());
}

#endif

#endif