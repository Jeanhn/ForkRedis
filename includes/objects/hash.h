#ifndef __HASH_H__
#define __HASH_H__

#include <unordered_map>
#include <vector>
#include <objects/object.h>
#include <objects/str.h>
#include <util.h>

namespace rds
{

    class Hash final : public Object
    {
    private:
        std::unordered_map<Str, Str, decltype(&StrHash)> data_map_{0xff, StrHash};

    public:
        template <typename T,
                  typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
        void Set(const Str &, T &&);
        auto Get(const Str &) -> Str;
        auto Exist(const Str &) -> bool;
        void Del(const Str &);
        auto Len() -> std::size_t;
        auto GetAll() -> std::vector<std::pair<Str, Str>>;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> &) override;

        CLASS_DEFAULT_DECLARE(Hash);
    };

} // namespace rds

#endif