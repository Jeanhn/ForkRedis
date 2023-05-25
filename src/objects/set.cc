#include <objects/set.h>
#include <set>

namespace rds
{

    auto Set::Add(Str data) -> bool
    {
        auto it = data_set_.insert(std::move(data));
        return it.second;
    }

    auto Set::Card() const -> std::size_t
    {
        return data_set_.size();
    }

    auto Set::IsMember(const Str &m) const -> bool
    {
        auto it = data_set_.find(m);
        return (it != data_set_.cend());
    }

    auto Set::Members() const -> std::vector<Str>
    {
        std::vector<Str> ret;
        for (auto &element : data_set_)
        {
            ret.push_back(element);
        }
        return ret;
    }

    auto Set::RandMember() const -> Str
    {
        if (data_set_.empty())
        {
            return {};
        }
        auto it = data_set_.begin();

        std::advance(it, static_cast<std::size_t>(std::rand()) % data_set_.size());
        return *(it);
    }

    auto Set::Pop() -> Str
    {
        if (data_set_.empty())
        {
            return {};
        }
        Str s(std::move(*(data_set_.cbegin())));
        data_set_.erase(data_set_.cbegin());
        return *(data_set_.cbegin());
    }

    void Set::Rem(const Str &m)
    {
        auto it = data_set_.find(m);
        if (it == data_set_.cend())
        {
            return;
        }
        data_set_.erase(it);
    }

    auto Set::GetObjectType() const -> ObjectType
    {
        return ObjectType::SET;
    }

    auto Set::EncodeValue() const -> std::string
    {
        std::string ret = BitsToString(data_set_.size());
        std::for_each(std::cbegin(data_set_), std::cend(data_set_), [&ret](const Str &s) mutable
                      { ret.append(s.EncodeValue()); });
        return ret;
    }

    void Set::DecodeValue(std::deque<char> *source)
    {
        data_set_.clear();
        std::size_t len = PeekSize(source);
        for (std::size_t i = 0; i < len; i++)
        {
            Str s;
            s.DecodeValue(source);
            data_set_.insert(std::move(s));
        }
    }

    auto Set::Diff(const Set &s) -> std::vector<Str>
    {
        std::vector<Str> ret;
        std::for_each(data_set_.cbegin(), data_set_.cend(), [&ret, &s](const Str &str) mutable
                      {
            if(!s.IsMember(str)){
                ret.push_back(str);
            } });
        return ret;
    }

    auto Set::Inter(const Set &s) -> std::vector<Str>
    {
        std::vector<Str> ret;
        std::for_each(data_set_.cbegin(), data_set_.cend(), [&ret, &s](const Str &str) mutable
                      {
            if(s.IsMember(str)){
                ret.push_back(str);
            } });
        return ret;
    }
} // namespace fds
