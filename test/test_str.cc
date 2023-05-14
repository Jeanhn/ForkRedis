#include <objects/str.h>
#include <gtest/gtest.h>
#include <vector>
#include <util.h>

TEST(StrTest, Consrtuct)
{
    using namespace rds;
    std::string temp("hello"), temp2("hello");
    Str s;
    Str s1(temp);
    ASSERT_EQ(temp, temp2);
    Str s2(std::move(temp));
    ASSERT_EQ(temp.empty(), true);

    Str s3(s2);
    ASSERT_EQ(s3, s2);

    Str s4(std::move(s3));
    ASSERT_EQ(s3.Empty(), true);
    ASSERT_EQ(s4, s2);
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
    dcd_str.DecodeValue(str_cache);

    ASSERT_EQ(dcd_str.GetEncodingType(), etyp);

    ASSERT_EQ(str, dcd_str) << "str:" << str.GetRaw() << '\n'
                            << "dcd:" << dcd_str.GetRaw() << std::endl;
}

TEST(StrTest, DISABLED_Mix)
{
    using namespace rds;
    std::deque<char> cache;
    std::vector<std::string> int_str_src;
    for (int i = 0xffff; i < 0xfffff; i++)
    {
        int_str_src.push_back(std::to_string(i));
    }
    for (auto &s : int_str_src)
    {
        EncodeInt(s, cache);
        Decode(s, cache, EncodingType::INT);
    }
    for (auto &s : int_str_src)
    {
        EncodeInt(s, cache);
    }
    for (auto &s : int_str_src)
    {
        Decode(s, cache, EncodingType::INT);
    }
}

TEST(StrTest, DISABLED_RawEncodeAndDecode)
{
    using namespace rds;
    std::deque<char> cache;
    std::vector<std::string> raw_str_src;
    for (int i = 0; i < 1000; i++)
    {
        std::string s;
        for (int j = 20; j < 100; j++)
        {
            s.push_back('a' + j % 26);
        }
        raw_str_src.push_back(s);
    }
    std::vector<std::string> int_str_src;
    for (int i = 0xffff; i < 0x2ffff; i++)
    {
        int_str_src.push_back(std::to_string(i));
    }

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
}

TEST(TestCompressDecompress, CompressAndDecompress)
{
    std::string raw{"flkjdhsalkfhdaskljhfdlkjsahfkdhafkjdhlakjshflkjsaflkjdhsalkfhdaskljhfdlkjsahfkdhafkjdhlakjshflkjsa"};
    std::string cmprs = rds::Compress(raw);
    std::cout << "after compress:" << cmprs.size() << std::endl;
    std::string dcmprs = rds::Decompress(cmprs);
    ASSERT_NE(dcmprs, cmprs);
    ASSERT_EQ(raw, dcmprs) << "dcprs size:" << dcmprs.size();
}

TEST(StrTest, MixCompress)
{
    using namespace rds;
    EnCompress();
    std::deque<char> cache;
    std::vector<std::string> raw_str_src;
    for (int i = 0; i < 1000; i++)
    {
        std::string s;
        for (int j = 20; j < 100; j++)
        {
            s.push_back('a' + j % 26);
        }
        raw_str_src.push_back(s);
    }
    std::vector<std::string> int_str_src;
    for (int i = 0xffff; i < 0x2ffff; i++)
    {
        int_str_src.push_back(std::to_string(i));
    }

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
}