#include <gtest/gtest.h>
#include <objects/list.h>

TEST(ListTest, Normal)
{
    using namespace rds;
    List l;
    for (int i = 0; i < 1000; i++)
    {
        Str s(std::to_string(i));
        l.PushBack(s);
    }

    std::string s = l.EncodeValue();
    std::deque<char> cache;
    std::copy(std::cbegin(s), std::cend(s), std::back_inserter(cache));

    List l2;
    l2.DecodeValue(cache);

    ASSERT_EQ(l2.Len(), l.Len());
    std::size_t len = l.Len();
    for (std::size_t i = 0; i < len; i++)
    {
        auto s = l.PopFront();
        auto s2 = l2.PopFront();
        ASSERT_EQ(s, s2);
    }
}