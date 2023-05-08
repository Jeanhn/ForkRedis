#include <objects/list.h>

namespace rds
{

    template <typename T, typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
    void List::PushFront(T &&data)
    {
        data_list_.push_front(std::forward<T>(data));
    }

    template <typename T, typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
    void List::PushBack(T &&data)
    {
        data_list_.push_back(std::forward<T>(data));
    }

    auto List::PopFront() -> Str
    {
        if (data_list_.empty())
        {
            return {};
        }
        Str &s = data_list_.front();
        Str ret(std::move(s));
        data_list_.pop_front();
        return ret;
    }

    auto List::PopBack() -> Str
    {
        if (data_list_.empty())
        {
            return {};
        }
        Str &s = data_list_.back();
        Str ret(std::move(s));
        data_list_.pop_back();
        return ret;
    }

    auto List::Index(std::size_t idx) const -> Str
    {
        if (idx >= data_list_.size())
        {
            return {};
        }
        auto it = data_list_.cbegin();
        std::advance(it, idx);
        return *it;
    }

    auto List::Len() const -> std::size_t
    {
        return data_list_.size();
    }

    auto List::Rem(const Str &data) -> bool
    {
        auto it = std::remove(data_list_.begin(), data_list_.end(), data);
        if (it == data_list_.end())
        {
            return false;
        }
        data_list_.erase(it);
        return true;
    }

    void List::Trim(std::size_t begin, std::size_t end)
    {
        auto b = data_list_.cbegin();
        auto e = b;
        if (begin >= end || begin >= data_list_.size())
        {
            return;
        }
        if (end >= data_list_.size())
        {
            end = data_list_.size();
        }
        std::advance(b, begin);
        std::advance(e, end);
        data_list_.erase(b, e);
    }

    auto List::GetObjectType() const -> ObjectType { return ObjectType::LIST; }

    auto List::EncodeValue() const -> std::string
    {
        std::string ret = BitsToString(data_list_.size());
        std::for_each(std::cbegin(data_list_), std::cend(data_list_), [&ret](const Str &s)
                      { ret.append(s.EncodeValue()); });
        return ret;
    }

    void List::DecodeValue(std::deque<char> &source)
    {
        std::size_t len = PeekSize(source);
        data_list_.clear();
        for (std::size_t i = 0; i < len; i++)
        {
            Str s;
            s.DecodeValue(source);
            data_list_.push_back(std::move(s));
        }
    }

} // namespace rds
