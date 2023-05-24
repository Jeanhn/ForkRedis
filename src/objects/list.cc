#include <objects/list.h>

namespace rds
{

    void List::PushFront(Str str)
    {
        data_list_.push_front(std::move(str));
    }

    void List::PushBack(Str str)
    {
        data_list_.push_back(std::move(str));
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
        if (data_list_.empty())
        {
            return false;
        }
        auto it = std::find(data_list_.cbegin(), data_list_.cend(), data);
        if (it == data_list_.end())
        {
            return false;
        }
        data_list_.erase(it);
        return true;
    }

    void List::Trim(int begin, int end)
    {
        if (data_list_.empty())
        {
            return;
        }
        auto legalRange = [size = data_list_.size()](int r) -> std::size_t
        {
            if (r >= 0)
            {
                return r % size;
            }
            int _r = -r;
            r += (_r / size + 1) * size;
            return static_cast<std::size_t>(r);
        };
        begin = legalRange(begin);
        end = legalRange(end);
        if (begin > end)
        {
            std::swap(begin, end);
        }
        auto beg = std::begin(data_list_);
        auto ed = beg;
        std::advance(beg, static_cast<std::size_t>(begin));
        std::advance(ed, static_cast<std::size_t>(end) + 1);
        data_list_.erase(beg, ed);
    }

    auto List::GetObjectType() const -> ObjectType { return ObjectType::LIST; }

    auto List::EncodeValue() const -> std::string
    {
        std::string ret = BitsToString(data_list_.size());
        std::for_each(std::cbegin(data_list_), std::cend(data_list_), [&ret](const Str &s) mutable
                      { ret.append(s.EncodeValue()); });
        return ret;
    }

    void List::DecodeValue(std::deque<char> *source)
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
