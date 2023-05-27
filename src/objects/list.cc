#include <objects/list.h>

namespace rds
{
    List::List(const List &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_list_ = lhs.data_list_;
    }

    List::List(List &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_list_ = std::move(rhs.data_list_);
    }

    List &List::operator=(const List &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_list_ = lhs.data_list_;
        return *this;
    }

    List &List::operator=(List &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_list_ = std::move(rhs.data_list_);
        return *this;
    }

    auto List::PushFront(Str str) -> std::size_t
    {
        WriteGuard wg(latch_);
        data_list_.push_front(std::move(str));
        return data_list_.size();
    }

    auto List::PushBack(Str str) -> std::size_t
    {
        WriteGuard wg(latch_);
        data_list_.push_back(std::move(str));
        return data_list_.size();
    }

    auto List::PopFront() -> Str
    {
        WriteGuard wg(latch_);
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
        WriteGuard wg(latch_);
        if (data_list_.empty())
        {
            return {};
        }
        Str &s = data_list_.back();
        Str ret(std::move(s));
        data_list_.pop_back();
        return ret;
    }

    auto List::Index(int idx) const -> Str
    {
        ReadGuard rg(latch_);
        if (data_list_.empty())
        {
            return {};
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
        idx = legalRange(idx);
        auto it = data_list_.cbegin();
        std::advance(it, idx);
        return *it;
    }

    auto List::Len() const -> std::size_t
    {
        ReadGuard rg(latch_);
        return data_list_.size();
    }

    auto List::Rem(int count, const Str &value) -> std::size_t
    {
        WriteGuard wg(latch_);
        if (data_list_.empty())
        {
            return 0;
        }
        std::size_t abs = count;
        if (count < 0)
        {
            abs = -count;
        }
        std::size_t cnt = std::count(data_list_.cbegin(), data_list_.cend(), value);
        if (!cnt == 0)
        {
            return 0;
        }
        if (cnt > abs)
        {
            cnt = abs;
        }
        auto n = cnt;
        if (count > 0)
        {
            auto beg = data_list_.begin();
            auto next = beg;
            next++;
            while (n > 0)
            {
                if (*beg == value)
                {
                    data_list_.erase(beg);
                    n--;
                    if (next == data_list_.end())
                    {
                        return cnt;
                    }
                }
                beg = next;
                next++;
            }
        }
        else
        {
            auto beg = data_list_.end();
            beg--;
            auto next = beg;
            if (next != data_list_.begin())
            {
                next--;
            }
            while (n > 0)
            {
                if (*beg == value)
                {
                    data_list_.erase(beg);
                    if (beg == next)
                    {
                        return cnt;
                    }
                    n--;
                }
                beg = next;
                if (next != data_list_.begin())
                {
                    next--;
                }
            }
        }
        return cnt;
    }

    auto List::Trim(int begin, int end) -> bool
    {
        WriteGuard wg(latch_);
        if (data_list_.empty())
        {
            return false;
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
            return false;
        }
        auto beg = std::begin(data_list_);
        auto ed = beg;
        std::advance(beg, static_cast<std::size_t>(begin));
        std::advance(ed, static_cast<std::size_t>(end) + 1);
        data_list_.erase(beg, ed);
        return true;
    }

    auto List::Set(int index, const Str &value) -> bool
    {
        WriteGuard wg(latch_);
        if (data_list_.empty())
        {
            return false;
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
        index = legalRange(index);
        auto it = data_list_.begin();
        std::advance(it, index);
        *it = value;
        return true;
    }

    auto List::GetObjectType() const -> ObjectType { return ObjectType::LIST; }

    auto List::EncodeValue() const -> std::string
    {
        ReadGuard rg(latch_);
        std::string ret = BitsToString(data_list_.size());
        std::for_each(std::cbegin(data_list_), std::cend(data_list_), [&ret](const Str &s) mutable
                      { ret.append(s.EncodeValue()); });
        return ret;
    }

    void List::DecodeValue(std::deque<char> *source)
    {
        WriteGuard wg(latch_);
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
