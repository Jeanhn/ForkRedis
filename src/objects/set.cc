#include <objects/set.h>

namespace rds
{

    template <typename T,
              typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
    void Set::Add(T &&data)
    {
        data_set_.insert(std::forward<T>(data));
    }

    auto Set::Card() -> std::size_t
    {
        return data_set_.size();
    }

    auto Set::IsMember(const Str &m) -> bool
    {
        auto it = data_set_.find(m);
        return (it == data_set_.cend());
    }

    auto Set::Members() -> std::vector<Str>
    {
        std::vector<Str> ret;
        for (auto &element : data_set_)
        {
            ret.push_back(element);
        }
        return ret;
    }

    auto Set::RandMember() -> Str
    {
        if (data_set_.empty())
        {
            return {};
        }
        return *(data_set_.cbegin());
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

} // namespace fds
