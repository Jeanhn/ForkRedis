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
        void Set(const Str &key, Str value);
        auto Get(const Str &) -> Str;
        auto Exist(const Str &) -> bool;
        void Del(const Str &);
        auto Len() -> std::size_t;
        auto GetAll() -> std::vector<std::pair<Str, Str>>;
        auto IncrBy(const Str &key, int delta) -> std::string;
        auto DecrBy(const Str &key, int delta) -> std::string;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> *) override;

        auto Fork(const std::string &key) -> std::string;

        CLASS_DECLARE_special_copy_move(Hash);
    };

} // namespace rds

#endif