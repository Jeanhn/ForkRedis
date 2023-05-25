#include <objects/zset.h>

namespace rds
{

    // ZSet::ZSet(ZSet &&rhs) : sequence_list_(rhs.sequence_list_), member_map_(std::move(rhs.member_map_)),
    //                          rank_map_(std::move(rank_map_)) {}
    // ZSet &ZSet::operator=(ZSet &&rhs) {
    //     sequence_list_ = rhs.sequence_list_;

    // }

    auto ZSet::Add(int score, Str member) -> bool
    {
        auto it = member_map_.insert({std::move(member), sequence_list_.end()});
        if (!it.second)
        {
            return false;
        }
        sequence_list_.push_back({&it.first->first, score});
        auto pos = sequence_list_.end();
        pos--;
        it.first->second = pos;
        rank_map_.insert({score, it.first});
        return true;
    }

    auto ZSet::Card() -> std::size_t
    {
        return sequence_list_.size();
    }

    auto ZSet::Rem(int score, const Str &member) -> bool
    {
        auto pos = member_map_.find(member);
        if (pos == member_map_.end())
        {
            return false;
        }
        if (pos->second->second != score)
        {
            return false;
        }
        sequence_list_.erase(pos->second);
        auto it = rank_map_.upper_bound(score - 1);
        while (it->second != pos)
        {
            it++;
        }
        rank_map_.erase(it);
        member_map_.erase(pos);
        return true;
    }

    auto ZSet::Count(int score_low, int score_high) -> std::size_t
    {
        if (score_low > score_high)
        {
            return 0;
        }
        auto low = rank_map_.lower_bound(score_low);
        auto high = rank_map_.upper_bound(score_high);
        return std::distance(low, high);
    }

    auto ZSet::LexCount(const Str &member_low, const Str &member_high) -> std::size_t
    {
        if (member_low < member_high)
        {
            return 0;
        }
        auto low = member_map_.lower_bound(member_low);
        auto high = member_map_.upper_bound(member_high);
        return std::distance(low, high);
    }

    auto ZSet::IncrBy(int delta_score, const Str &member) -> bool
    {
        auto pos = member_map_.find(member);
        if (pos == member_map_.end())
        {
            return false;
        }
        auto it = rank_map_.lower_bound(pos->second->second);
        while (it->second != pos)
        {
            it++;
        }
        rank_map_.erase(it);
        rank_map_.insert({pos->second->second + delta_score, pos});
        pos->second->second += delta_score;
        return true;
    }

    auto ZSet::DecrBy(int delta_score, const Str &member) -> bool
    {
        return IncrBy(-delta_score, member);
    }

    auto ZSet::Range(int beg, int end) -> std::vector<std::pair<Str, int>>
    {
        if (sequence_list_.empty())
        {
            return {};
        }
        auto legalRange = [size = sequence_list_.size()](int r) -> std::size_t
        {
            if (r >= 0)
            {
                return r % size;
            }
            int _r = -r;
            r += (_r / size + 1) * size;
            return static_cast<std::size_t>(r);
        };
        std::vector<std::pair<Str, int>> ret;
        std::size_t lbeg = legalRange(beg);
        std::size_t lend = legalRange(end);
        if (lbeg >= lend)
        {
            return {};
        }
        auto b = sequence_list_.cbegin();
        auto e = b;
        std::advance(b, lbeg);
        std::advance(e, lend + 1);
        std::for_each(b, e, [&ret](const decltype(sequence_list_)::value_type &v)
                      { ret.push_back({*(v.first), v.second}); });
        return ret;
    }

    auto ZSet::RangeByScore(int score_low, int score_high) -> std::vector<std::pair<Str, int>>
    {
        if (score_low > score_high)
        {
            return {};
        }
        auto low = rank_map_.lower_bound(score_low);
        auto high = rank_map_.upper_bound(score_high);
        std::vector<std::pair<Str, int>> ret;
        std::for_each(low, high, [&ret](const decltype(rank_map_)::value_type &v) mutable
                      { ret.push_back({v.second->first, v.first}); });
        return ret;
    }

    auto ZSet::RangeByLex(const Str &member_low, const Str &member_high) -> std::vector<std::pair<Str, int>>
    {
        if (member_low > member_high)
        {
            return {};
        }
        std::vector<std::pair<Str, int>> ret;
        auto low = member_map_.lower_bound(member_low);
        auto high = member_map_.upper_bound(member_high);
        std::for_each(low, high, [&ret](const decltype(member_map_)::value_type &v) mutable
                      { ret.push_back({v.first, v.second->second}); });
        return ret;
    }

    auto ZSet::GetObjectType() const -> ObjectType
    {
        return ObjectType::ZSET;
    }

    auto ZSet::EncodeValue() const -> std::string
    {
        std::string ret = BitsToString(sequence_list_.size());
        std::for_each(std::cbegin(sequence_list_), std::cend(sequence_list_), [&ret](const decltype(sequence_list_)::value_type &v) mutable
                      { ret.append(BitsToString(v.second));ret.append(v.first->EncodeValue()); });
        return ret;
    }

    void ZSet::DecodeValue(std::deque<char> *source)
    {
        member_map_.clear();
        std::size_t len = PeekSize(source);
        for (std::size_t i = 0; i < len; i++)
        {
            Str s;
            int r;
            r = PeekInt(source);
            s.DecodeValue(source);
            Add(r, std::move(s));
        }
    }
} // namespace rds
