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
        bool compress_{false};

        void TypeCheck();

    public:
        auto GetRaw() -> std::string &;
        void Set(const std::string &);
        void Set(std::string &&);
        void Append(const std::string &);
        void Append(std::string &&);
        void IncrBy(int);
        void DecrBy(int);
        auto Len() const -> std::size_t;
        auto Empty() const -> bool;

        auto GetEncodingType() const -> EncodingType;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> &) override;

        CLASS_DEFAULT_DECLARE(Str);

        template <typename T, typename =
                                  std::enable_if_t<std::is_same_v<std::string, std::decay_t<T>>, void>>
        Str(T &&data) : data_(std::forward<T>(data)), encoding_type_(EncodingType::INT)
        {
            for (auto it = data_.cbegin(); it != data_.cend(); it++)
            {
                if (*it < '0' || *it > '9')
                {
                    encoding_type_ = EncodingType::STR_RAW;
                }
            }
        }
    };

    inline auto operator==(const Str &a, const Str &b) -> bool
    {
        return a.data_ == b.data_;
    }
    inline auto operator<(const Str &a, const Str &b) -> bool
    {
        return a.data_ < b.data_;
    }
    inline auto operator<=(const Str &a, const Str &b) -> bool
    {
        return a.data_ <= b.data_;
    }
    inline auto operator>(const Str &a, const Str &b) -> bool
    {
        return a.data_ > b.data_;
    }
    inline auto operator>=(const Str &a, const Str &b) -> bool
    {
        return a.data_ >= b.data_;
    }

    inline auto StrLess(const Str &a, const Str &b) -> bool
    {
        return a < b;
    }

    inline auto StrHash(const Str &s) -> std::size_t
    {
        static std::hash<std::string> hash_;
        return hash_(s.data_);
    }

} // namespace rds

#endif