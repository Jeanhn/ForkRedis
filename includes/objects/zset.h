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
        auto Add(int, Str) -> bool;
        auto Card() -> std::size_t;
        auto Rem(int, const Str &) -> bool;
        auto Count(int, int) -> std::size_t;
        auto LexCount(const Str &, const Str &) -> std::size_t;
        auto IncrBy(int, const Str &) -> bool;
        auto DecrBy(int, const Str &) -> bool;
        auto Range(int, int) -> std::vector<std::pair<Str, int>>;
        auto RangeByScore(int, int) -> std::vector<std::pair<Str, int>>;
        auto RangeByLex(const Str &, const Str &) -> std::vector<std::pair<Str, int>>;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> *) override;

        CLASS_DEFAULT_DECLARE(ZSet);
        // ZSet() = default;
        // ZSet(const ZSet &) = default;
        // ~ZSet() = default;
        // ZSet &operator=(const ZSet &) = default;
        // ZSet(ZSet &&);
        // ZSet &operator=(ZSet &&);
    };

} // namespace rds

#endif