#include <objects/str.h>
#include <gtest/gtest.h>
#include <vector>
#include <util.h>
#include <objects/list.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <objects/hash.h>
#include <algorithm>

void CheckWhat(const std::string &what)
{
    std::cout << "check: " << what << std::endl;
}

void EncodeInt(const std::string &int_str, std::deque<char> &int_cache)
{
    using namespace rds;

    Str istr(int_str);

    ASSERT_EQ(istr.GetEncodingType(), EncodingType::INT);

    std::string int_encode = istr.EncodeValue();

    ASSERT_EQ(int_encode.size(), sizeof(int) + sizeof(char));

    int_cache.insert(int_cache.end(), int_encode.cbegin(), int_encode.cend());
}

void EncodeRaw(const std::string &raw_str, std::deque<char> &str_cache)
{
    using namespace rds;

    Str rstr(raw_str);

    ASSERT_EQ(rstr.GetEncodingType(), EncodingType::STR_RAW);

    std::string str_encode = rstr.EncodeValue();

    if (!DefineCompress())
    {
        ASSERT_EQ(str_encode.size(), raw_str.size() + sizeof(size_t) + sizeof(char));
    }

    str_cache.insert(str_cache.end(), str_encode.cbegin(), str_encode.cend());
}

void Decode(const std::string &data, std::deque<char> &str_cache, rds::EncodingType etyp)
{
    using namespace rds;

    Str str(data);

    Str dcd_str;
    dcd_str.DecodeValue(&str_cache);

    ASSERT_EQ(dcd_str.GetEncodingType(), etyp);

    ASSERT_EQ(str, dcd_str) << "str:" << str.GetRaw() << '\n'
                            << "dcd:" << dcd_str.GetRaw() << std::endl;
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

TEST(Structs, CompressAndDecompress)
{
    std::string raw{"flkjdhsalkfhdaskljhfdlkjsahfkdhafkjdhlakjshflkjsaflkjdhsalkfhdaskljhfdlkjsahfkdhafkjdhlakjshflkjsa"};
    std::string cmprs = rds::Compress(raw);
    std::cout << "after compress:" << cmprs.size() << std::endl;
    std::string dcmprs = rds::Decompress(cmprs);
    ASSERT_NE(dcmprs, cmprs);
    ASSERT_EQ(raw, dcmprs) << "dcprs size:" << dcmprs.size();

    CheckWhat("third-lzf");
    CheckWhat("\n");
}

TEST(Structs, Str)
{
    using namespace rds;
    EnCompress();
    std::deque<char> cache;

    auto raw_str_src = RandStrArr(100);
    auto int_str_src = RandIntArr(100);

    for (auto &s : raw_str_src)
    {
        EncodeRaw(s, cache);
    }

    for (auto &s : int_str_src)
    {
        EncodeInt(s, cache);
    }

    for (auto &s : raw_str_src)
    {
        Decode(s, cache, EncodingType::STR_RAW);
    }
    for (auto &s : int_str_src)
    {
        Decode(s, cache, EncodingType::INT);
    }

    std::vector<std::string> dt{"a", "b", "c", "d", "e", "f"};
    std::vector<Str> data;
    for (auto s : dt)
    {
        data.push_back(s);
    }
    std::string c("c");
    auto pos = std::lower_bound(data.cbegin(), data.cend(), c);
    ASSERT_EQ(pos, data.cbegin() + 2);

    CheckWhat("str en-de-code");
    CheckWhat("\n");
}

TEST(Structs, List)
{
    using namespace rds;
    List l;
    EnCompress();
    for (int i = 0; i < 1000; i++)
    {
        Str s(std::to_string(i));
        l.PushBack(s);
    }

    std::string s = l.EncodeValue();
    std::deque<char> cache;
    std::copy(std::cbegin(s), std::cend(s), std::back_inserter(cache));

    List l2;
    l2.DecodeValue(&cache);

    ASSERT_EQ(l2.Len(), l.Len());
    std::size_t len = l.Len();
    for (std::size_t i = 0; i < len; i++)
    {
        auto s = l.PopFront();
        auto s2 = l2.PopFront();
        ASSERT_EQ(s, s2);
    }
    CheckWhat("list en-de-code");

    List li;
    std::vector<Str> vec;
    for (int i = 0; i < 1000; i++)
    {
        li.PushBack(std::to_string(i));
        vec.push_back(std::to_string(i));
    }

    li.Trim(0, -500 - li.Len() * 2);
    ASSERT_EQ(li.Len(), 499);
    vec.erase(vec.begin(), vec.begin() + 501);

    for (int i = 0; i < 499; i++)
    {
        auto s = li.PopFront();
        ASSERT_EQ(s, vec[i]) << "faied at:" << i << std::endl;
    }
    CheckWhat("list trim");
    CheckWhat("\n");
}

TEST(Structs, Set)
{
    using namespace rds;
    Set s;
    EnCompress();
    for (int i = 0; i < 1000; i++)
    {
        Str str(std::to_string(i));
        s.Add(str);
    }
    std::string ev = s.EncodeValue();
    std::deque<char> cache;
    std::copy(std::cbegin(ev), std::cend(ev), std::back_inserter(cache));

    Set s2;
    s2.DecodeValue(&cache);

    ASSERT_EQ(s.Card(), s2.Card()) << "orgin size\n";

    CheckWhat("set en-de-code");

    auto v2 = s2.Members();
    auto v = s.Members();
    ASSERT_EQ(v2.size(), 1000);
    ASSERT_EQ(v.size(), 1000);

    std::sort(v.begin(), v.end());
    std::sort(v2.begin(), v2.end());

    for (int i = 0; i < 1000; i++)
    {
        ASSERT_EQ(v[i], v2[i]);
    }
    CheckWhat("set members");

    Set s3, s4;
    for (int i = 0; i < 300; i++)
    {
        if (i < 200)
        {
            s3.Add({std::to_string(i)});
        }
        if (i > 100)
        {
            s4.Add({std::to_string(i)});
        }
    }
    auto diff = s3.Diff(s4);
    ASSERT_EQ(diff.size(), 101) << "diff size";
    CheckWhat("set diff");

    for (int i = 0; i < 101; i++)
    {
        // ASSERT_NE(diff.find({std::to_string(i)}), diff.end());
    }

    auto inter = s3.Inter(s4);
    ASSERT_EQ(inter.size(), 99);
    for (int i = 101; i < 200; i++)
    {
        // ASSERT_NE(inter.find({std::to_string(i)}), inter.end());
    }
    CheckWhat("set inter");
    CheckWhat("\n");
}

TEST(Structs, ZSet)
{
    using namespace rds;
    ZSet s;
    for (int i = 0; i < 1000; i++)
    {
        Str str(std::to_string(i));
        s.Add(i, str);
    }
    std::string ev = s.EncodeValue();
    std::deque<char> cache;
    std::copy(std::cbegin(ev), std::cend(ev), std::back_inserter(cache));
    Log("zset construct");

    ZSet s2;
    s2.DecodeValue(&cache);
    Log("zset decode");
    /* check en-de-code */
    ASSERT_EQ(s.Card(), s2.Card());
    CheckWhat("zset en-de-code");

    std::size_t tot = 0, delta = 0;
    tot = s.Card();

    /* check rem */
    for (int i = 700; i < 1000; i++)
    {
        delta++;
        s.Rem(i, {std::to_string(i)});
    }

    ASSERT_EQ(s.Card(), tot - delta);

    auto range_member = s.Range(0, -1);
    auto range_member_score = s.RangeByScore(0, 700);
    auto range_member_lex = s.RangeByLex({std::to_string(0)}, {std::to_string(700)});

    ASSERT_EQ(s.Card(), 700);
    ASSERT_EQ(range_member.size(), 700);
    ASSERT_EQ(range_member_score.size(), 700);
    std::size_t lexnum = 0;
    for (int i = 0; i < 700; i++)
    {
        auto q = std::to_string(i);
        if (q <= "700" && q >= "0")
        {
            lexnum++;
        }
    }
    ASSERT_EQ(range_member_lex.size(), lexnum);
    ASSERT_EQ(std::is_sorted(range_member_lex.cbegin(), range_member_lex.cend()), true);

    for (int i = 0; i < 700; i++)
    {
        Str temp(std::to_string(i));
        ASSERT_EQ(range_member[i].first, temp);
        // ASSERT_EQ(range_member_lex[i].first, temp);
        ASSERT_EQ(range_member_score[i].first, temp);
    }
    CheckWhat("zset rem");

    for (int i = 0; i < 1000; i++)
    {
        s2.IncrBy(1, {std::to_string(i)});
    }
    auto member2 = s2.Range(0, -1);
    auto member2_lex = s2.RangeByLex({std::string("0")}, {std::string("999")});
    auto member2_score = s2.RangeByScore(0, 1000);
    auto checkSet = [](decltype(member2) &member_set)
    {
        for (auto &e : member_set)
        {
            ASSERT_EQ(e.second, std::stoi(e.first.GetRaw()) + 1);
        }
    };
    checkSet(member2);
    checkSet(member2_lex);
    checkSet(member2_score);
    CheckWhat("zset incr");

    ZSet zs;
    for (int i = 0; i < 10; i++)
    {
        zs.Add(i, std::to_string(i));
    }
    auto lexcnt = zs.LexCount(std::string("0"), std::string("5"));
    ASSERT_EQ(lexcnt, 6);

    CheckWhat("\n");
}

TEST(Structs, Hash)
{
    using namespace rds;
    Hash s;
    for (int i = 0; i < 1000; i++)
    {
        Str str(std::to_string(i));
        s.Set(str, str);
    }
    std::string ev = s.EncodeValue();
    std::deque<char> cache;
    std::copy(std::cbegin(ev), std::cend(ev), std::back_inserter(cache));

    Hash s2;
    s2.DecodeValue(&cache);

    auto v2 = s2.GetAll();
    auto v = s.GetAll();
    ASSERT_EQ(v2.size(), 1000);
    ASSERT_EQ(v.size(), 1000);

    std::sort(v.begin(), v.end());
    std::sort(v2.begin(), v2.end());

    for (int i = 0; i < 1000; i++)
    {
        ASSERT_EQ(v[i], v2[i]);
    }
    CheckWhat("hash en-de-code");
    CheckWhat("\n");
}