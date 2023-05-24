#include <database/db.h>
#include <gtest/gtest.h>
#include <objects/str.h>
#include <objects/set.h>

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
    kv.MakeExpire(100);

    std::deque<char> src;
    std::string code = kv.Encode();
    std::copy(code.cbegin(), code.cend(), std::back_inserter(src));

    KeyValue kv2;
    kv2.Decode(&src);

    ASSERT_EQ(kv2.GetExpire().value(), 100);

    ASSERT_EQ(kv2.GetKey(), kv.GetKey());

    auto _v1 = kv.GetValue();
    auto _v2 = kv.GetValue();

    ASSERT_EQ(_v1->GetObjectType(), _v2->GetObjectType());

    auto sv = reinterpret_cast<Set *>(_v1);
    auto sv2 = reinterpret_cast<Set *>(_v2);

    auto m = sv->Members();
    auto m2 = sv2->Members();

    std::sort(m.begin(), m.end());
    std::sort(m2.begin(), m2.end());

    ASSERT_EQ(m, m2);
    CheckWhat("kv single en-de-code");

    std::vector<KeyValue> kvec;
    for (int i = 0; i < 1000; i++)
    {
        Str s(std::to_string(i));
        auto sp = std::make_unique<Str>(s);
        kvec.push_back({s, std::move(sp)});
    }
    // decltype(kvec) kvec2;
}