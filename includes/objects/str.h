#ifndef __STR_H__
#define __STR_H__

#include <objects/object.h>
#include <string>

namespace rds
{

    class Str final : public Object
    {
        friend auto operator==(const Str &a, const Str &b) -> bool;
        friend auto operator<(const Str &a, const Str &b) -> bool;
        friend auto operator<=(const Str &a, const Str &b) -> bool;
        friend auto operator>(const Str &a, const Str &b) -> bool;
        friend auto operator>=(const Str &a, const Str &b) -> bool;

        friend auto StrHash(const Str &s) -> std::size_t;

        friend auto StrLess(const Str &a, const Str &b) -> bool;

    private:
        std::string data_;
        EncodingType encoding_type_;
        bool compress_;

    public:
        auto GetRaw() -> std::string &;
        void Set(const std::string &);
        void Set(std::string &&);
        void Append(const std::string &);
        void Append(std::string &&);
        void IncrBy(int);
        void DecrBy(int);
        auto Len() -> std::size_t;
        auto Empty() -> bool;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() -> std::string override;
        void DecodeValue(std::deque<char> &) override;

        CLASS_DEFAULT_DECLARE(Str);

        template <typename T, typename =
                                  std::enable_if_t<std::is_same_v<std::string, std::decay_t<T>>, void>>
        Str(T &&);
    };

    auto operator==(const Str &a, const Str &b) -> bool
    {
        return a.data_ == b.data_;
    }
    auto operator<(const Str &a, const Str &b) -> bool
    {
        return a.data_ < b.data_;
    }
    auto operator<=(const Str &a, const Str &b) -> bool
    {
        return a.data_ <= b.data_;
    }
    auto operator>(const Str &a, const Str &b) -> bool
    {
        return a.data_ > b.data_;
    }
    auto operator>=(const Str &a, const Str &b) -> bool
    {
        return a.data_ >= b.data_;
    }

    auto StrLess(const Str &a, const Str &b) -> bool
    {
        return a < b;
    }

    auto StrHash(const Str &s) -> std::size_t
    {
        static std::hash<std::string> hash_;
        return hash_(s.data_);
    }

} // namespace rds

#endif