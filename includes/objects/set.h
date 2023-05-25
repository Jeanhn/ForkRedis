#ifndef __SET_H__
#define __SET_H__

#include <objects/object.h>
#include <util.h>
#include <objects/str.h>
#include <unordered_set>

namespace rds
{

    class Set final : public Object
    {
    private:
        std::unordered_set<Str, decltype(&StrHash)> data_set_{0xff, StrHash};

    public:
        auto GetRawSet() -> std::unordered_set<Str, decltype(&StrHash)> &;
        auto Add(Str data) -> bool;
        auto Card() const -> std::size_t;
        auto IsMember(const Str &) const -> bool;
        auto Members() const -> std::vector<Str>;
        auto RandMember() const -> Str;
        auto Pop() -> Str;
        void Rem(const Str &);
        auto Diff(const Set &) -> std::vector<Str>;
        auto Inter(const Set &) -> std::vector<Str>;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> *) override;

        CLASS_DEFAULT_DECLARE(Set);
    };

} // namespace rds

#endif