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
        template <typename T,
                  typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
        void Add(T &&);
        auto Card() const -> std::size_t;
        auto IsMember(const Str &) const -> bool;
        auto Members() const -> std::vector<Str>;
        auto RandMember() const -> Str;
        auto Pop() -> Str;
        void Rem(const Str &);

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> &) override;

        CLASS_DEFAULT_DECLARE(Set);
    };

} // namespace rds

#endif