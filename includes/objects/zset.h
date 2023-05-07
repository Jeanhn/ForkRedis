#ifndef __ZSET_H__
#define __ZSET_H__

#include <objects/object.h>
#include <util.h>
#include <objects/str.h>
#include <set>

namespace rds
{

    class ZSet final : public Object
    {
    private:
        std::set<Str, decltype(&StrLess)> data_set_{StrLess};

    public:
        template <typename T,
                  typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
        void Add(T &&);
        auto Card() -> std::size_t;
        auto IsMember(const Str &) -> bool;
        auto Members() -> std::vector<Str>;
        auto RandMember() -> Str;
        auto Pop() -> Str;
        void Rem(const Str &);

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() -> std::string override;
        void DecodeValue(std::deque<char> &) override;

        CLASS_DEFAULT_DECLARE(ZSet);
    };

} // namespace rds

#endif