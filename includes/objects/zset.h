#ifndef __ZSET_H__
#define __ZSET_H__

#include <objects/object.h>
#include <util.h>
#include <objects/str.h>
#include <map>
#include <list>

namespace rds
{

    class ZSet final : public Object
    {
    private:
        std::list<std::pair<const Str *, int>> sequence_list_;
        std::map<Str, decltype(sequence_list_)::iterator> member_map_;
        std::multimap<int, decltype(member_map_)::iterator> rank_map_;

    public:
        auto Add(int, Str) -> bool; // number of new (except update)
        auto Card() const -> std::size_t;
        auto Rem(int, const Str &) -> bool;
        auto Count(int, int) const -> std::size_t;
        auto LexCount(const Str &, const Str &) const -> std::size_t;
        auto IncrBy(int, const Str &) -> std::string;
        auto DecrBy(int, const Str &) -> std::string;
        auto Range(int, int) const -> std::vector<std::pair<Str, int>>;
        auto RangeByScore(int, int) const -> std::vector<std::pair<Str, int>>;
        auto RangeByLex(const Str &, const Str &) const -> std::vector<std::pair<Str, int>>;
        auto Rank(const Str &member) const -> std::string;
        auto Score(const Str &member) const -> std::string;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> *) override;

        CLASS_DECLARE_special_copy_move(ZSet);
    };

} // namespace rds

#endif