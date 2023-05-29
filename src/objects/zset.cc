#include <objects/zset.h>

namespace rds
{

    ZSet::ZSet(const ZSet &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        for (auto member : lhs.sequence_list_)
        {
            Add(member.second, *(member.first));
        }
    }

    ZSet::ZSet(ZSet &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        sequence_list_ = std::move(rhs.sequence_list_);
        member_map_ = std::move(rhs.member_map_);
        rank_map_ = std::move(rhs.rank_map_);
    }

    ZSet &ZSet::operator=(const ZSet &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        for (auto member : lhs.sequence_list_)
        {
            Add(member.second, *(member.first));
        }
        return *this;
    }

    ZSet &ZSet::operator=(ZSet &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        sequence_list_ = std::move(rhs.sequence_list_);
        member_map_ = std::move(rhs.member_map_);
        rank_map_ = std::move(rhs.rank_map_);
        return *this;
    }

    auto ZSet::Add(int score, Str member) -> bool
    {
        WriteGuard wg(latch_);
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

    auto ZSet::Card() const -> std::size_t
    {
        ReadGuard rg(latch_);
        return sequence_list_.size();
    }

    auto ZSet::Rem(int score, const Str &member) -> bool
    {
        WriteGuard wg(latch_);
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

    auto ZSet::Count(int score_low, int score_high) const -> std::size_t
    {
        if (score_low > score_high)
        {
            return 0;
        }
        ReadGuard rg(latch_);
        auto low = rank_map_.lower_bound(score_low);
        auto high = rank_map_.upper_bound(score_high);
        return std::distance(low, high);
    }

    auto ZSet::LexCount(const Str &member_low, const Str &member_high) const -> std::size_t
    {
        if (member_low > member_high)
        {
            return 0;
        }
        ReadGuard rg(latch_);
        auto low = member_map_.lower_bound(member_low);
        if (low != member_map_.end())
        {
            std::cout << low->first.GetRaw() << std::endl;
        }
        auto high = member_map_.upper_bound(member_high);
        if (high != member_map_.end())
        {
            std::cout << high->first.GetRaw() << std::endl;
        }
        return std::distance(low, high);
    }

    auto ZSet::IncrBy(int delta_score, const Str &member) -> std::string
    {
        WriteGuard wg(latch_);
        auto pos = member_map_.find(member);
        if (pos == member_map_.end())
        {
            return {};
        }
        auto it = rank_map_.lower_bound(pos->second->second);
        while (it->second != pos)
        {
            it++;
        }
        rank_map_.erase(it);
        rank_map_.insert({pos->second->second + delta_score, pos});
        pos->second->second += delta_score;
        return std::to_string(pos->second->second);
    }

    auto ZSet::DecrBy(int delta_score, const Str &member) -> std::string
    {
        return IncrBy(-delta_score, member);
    }

    auto ZSet::Range(int beg, int end) const -> std::vector<std::pair<Str, int>>
    {
        ReadGuard rg(latch_);
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

    auto ZSet::RangeByScore(int score_low, int score_high) const -> std::vector<std::pair<Str, int>>
    {
        if (score_low > score_high)
        {
            return {};
        }
        ReadGuard rg(latch_);
        auto low = rank_map_.lower_bound(score_low);
        auto high = rank_map_.upper_bound(score_high);
        std::vector<std::pair<Str, int>> ret;
        std::for_each(low, high, [&ret](const decltype(rank_map_)::value_type &v) mutable
                      { ret.push_back({v.second->first, v.first}); });
        return ret;
    }

    auto ZSet::RangeByLex(const Str &member_low, const Str &member_high) const -> std::vector<std::pair<Str, int>>
    {
        if (member_low > member_high)
        {
            return {};
        }
        ReadGuard rg(latch_);
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
        ReadGuard rg(latch_);
        std::string ret = BitsToString(sequence_list_.size());
        std::for_each(std::cbegin(sequence_list_), std::cend(sequence_list_), [&ret](const decltype(sequence_list_)::value_type &v) mutable
                      { ret.append(BitsToString(v.second));ret.append(v.first->EncodeValue()); });
        return ret;
    }

    void ZSet::DecodeValue(std::deque<char> *source)
    {
        WriteGuard wg(latch_);
        member_map_.clear();
        std::size_t len = PeekSize(source);
        for (std::size_t i = 0; i < len; i++)
        {
            Str s;
            int r;
            r = PeekInt(source);
            s.DecodeValue(source);

            auto it = member_map_.insert({std::move(s), sequence_list_.end()});
            if (!it.second)
            {
                continue;
            }
            sequence_list_.push_back({&it.first->first, r});
            auto pos = sequence_list_.end();
            pos--;
            it.first->second = pos;
            rank_map_.insert({r, it.first});
        }
    }

    auto ZSet::Rank(const Str &member) const -> std::string
    {
        ReadGuard rg(latch_);
        auto pos = member_map_.find(member);
        if (pos == member_map_.end())
        {
            return {};
        }
        auto rank_pos = rank_map_.find(pos->second->second);
        return std::to_string(std::distance(rank_map_.begin(), rank_pos));
    }

    auto ZSet::Score(const Str &member) const -> std::string
    {
        ReadGuard rg(latch_);
        auto pos = member_map_.find(member);
        if (pos == member_map_.end())
        {
            return {};
        }
        return std::to_string(pos->second->second);
    }

    auto ZSet::Fork(const std::string &key) -> std::string
    {
        std::string ret("ZADD " + key + " ");
        ReadGuard rg(latch_);
        for (auto &p : sequence_list_)
        {
            ret.append(std::to_string(p.second) + " " + p.first->GetRaw() + " ");
        }
        return ret;
    }

} // namespace rds
