#include <objects/set.h>
#include <set>

namespace rds
{
    Set::Set(const Set &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_set_ = lhs.data_set_;
    }

    Set::Set(Set &&rhs)
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_set_ = std::move(rhs.data_set_);
    }

    Set &Set::operator=(const Set &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_set_ = lhs.data_set_;
        return *this;
    }

    Set &Set::operator=(Set &&rhs)
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_set_ = std::move(rhs.data_set_);
        return *this;
    }

    auto Set::Add(Str data) -> bool
    {
        WriteGuard wg(latch_);
        auto it = data_set_.insert(std::move(data));
        return it.second;
    }

    auto Set::Card() const -> std::size_t
    {
        ReadGuard rg(latch_);
        return data_set_.size();
    }

    auto Set::IsMember(const Str &m) const -> bool
    {
        ReadGuard rg(latch_);
        auto it = data_set_.find(m);
        return (it != data_set_.cend());
    }

    auto Set::Members() const -> std::vector<Str>
    {
        ReadGuard rg(latch_);
        std::vector<Str> ret;
        for (auto &element : data_set_)
        {
            ret.push_back(element);
        }
        return ret;
    }

    auto Set::RandMember() const -> Str
    {
        ReadGuard rg(latch_);
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
        WriteGuard wg(latch_);
        if (data_set_.empty())
        {
            return {};
        }
        Str s(std::move(*(data_set_.cbegin())));
        data_set_.erase(data_set_.cbegin());
        return *(data_set_.cbegin());
    }

    auto Set::Rem(const Str &m) -> bool
    {
        WriteGuard wg(latch_);
        auto it = data_set_.find(m);
        if (it == data_set_.cend())
        {
            return false;
        }
        data_set_.erase(it);
        return true;
    }

    auto Set::GetObjectType() const -> ObjectType
    {
        return ObjectType::SET;
    }

    auto Set::EncodeValue() const -> std::string
    {
        ReadGuard rg(latch_);
        std::string ret = BitsToString(data_set_.size());
        std::for_each(std::cbegin(data_set_), std::cend(data_set_), [&ret](const Str &s) mutable
                      { ret.append(s.EncodeValue()); });
        return ret;
    }

    void Set::DecodeValue(std::deque<char> *source)
    {
        WriteGuard wg(latch_);
        data_set_.clear();
        std::size_t len = PeekSize(source);
        for (std::size_t i = 0; i < len; i++)
        {
            Str s;
            s.DecodeValue(source);
            data_set_.insert(std::move(s));
        }
    }

    auto Set::Diff(const Set &s) const -> std::vector<Str>
    {
        if (this < &s)
        {
            ReadGuard rg(latch_);
            ReadGuard rgs(s.latch_);
            std::vector<Str> ret;
            std::for_each(data_set_.cbegin(), data_set_.cend(), [&ret, &s](const Str &str) mutable
                          {
            if(s.data_set_.find(str)==s.data_set_.end()){
                ret.push_back(str);
            } });
            return ret;
        }
        else if (this > &s)
        {
            ReadGuard rgs(s.latch_);
            ReadGuard rg(latch_);
            std::vector<Str> ret;
            std::for_each(data_set_.cbegin(), data_set_.cend(), [&ret, &s](const Str &str) mutable
                          {
            if(s.data_set_.find(str)==s.data_set_.end()){
                ret.push_back(str);
            } });
            return ret;
        }
        return {};
    }

    auto Set::Inter(const Set &s) const -> std::vector<Str>
    {
        if (this < &s)
        {
            ReadGuard rg(latch_);
            ReadGuard rgs(s.latch_);
            std::vector<Str> ret;
            std::for_each(data_set_.cbegin(), data_set_.cend(), [&ret, &s](const Str &str) mutable
                          {
            if(s.data_set_.find(str)!=s.data_set_.end()){
                ret.push_back(str);
            } });
            return ret;
        }
        else if (this > &s)
        {
            ReadGuard rgs(s.latch_);
            ReadGuard rg(latch_);
            std::vector<Str> ret;
            std::for_each(data_set_.cbegin(), data_set_.cend(), [&ret, &s](const Str &str) mutable
                          {
            if(s.data_set_.find(str)!=s.data_set_.end()){
                ret.push_back(str);
            } });
            return ret;
        }
        ReadGuard rg(latch_);
        std::vector<Str> ret;
        std::for_each(data_set_.cbegin(), data_set_.cend(), [&ret, &s](const Str &str) mutable
                      { ret.push_back(str); });
        return ret;
    }
} // namespace fds
