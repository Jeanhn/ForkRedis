#ifndef __STR_H__
#define __STR_H__

#include <objects/object.h>
#include <string>
#include <configure.h>

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

    public:
        auto GetRaw() const -> std::string;
        void Set(std::string);
        auto Append(std::string) -> std::size_t;
        auto IncrBy(int) -> std::string;
        auto DecrBy(int) -> std::string;
        auto Len() const -> std::size_t;
        auto Empty() const -> bool;

        auto GetEncodingType() const -> EncodingType;

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> *) override;

        auto Fork(const std::string &key) -> std::string;

        // auto Size() -> std::size_t;

        CLASS_DECLARE_special_copy_move(Str);

        Str(std::string data);
    };

    inline auto operator==(const Str &a, const Str &b) -> bool
    {
        if (&a < &b)
        {
            ReadGuard(a.ExposeLatch());
            ReadGuard(b.ExposeLatch());
            return a.data_ == b.data_;
        }
        else if (&a > &b)
        {
            ReadGuard(b.ExposeLatch());
            ReadGuard(a.ExposeLatch());
            return a.data_ == b.data_;
        }
        return true;
    }
    inline auto operator<(const Str &a, const Str &b) -> bool
    {
        if (&a < &b)
        {
            ReadGuard(a.ExposeLatch());
            ReadGuard(b.ExposeLatch());
            return a.data_ < b.data_;
        }
        else if (&a > &b)
        {
            ReadGuard(b.ExposeLatch());
            ReadGuard(a.ExposeLatch());
            return a.data_ < b.data_;
        }
        return false;
    }
    inline auto operator<=(const Str &a, const Str &b) -> bool
    {
        if (&a < &b)
        {
            ReadGuard(a.ExposeLatch());
            ReadGuard(b.ExposeLatch());
            return a.data_ <= b.data_;
        }
        else if (&a > &b)
        {
            ReadGuard(b.ExposeLatch());
            ReadGuard(a.ExposeLatch());
            return a.data_ <= b.data_;
        }
        return true;
    }
    inline auto operator>(const Str &a, const Str &b) -> bool
    {
        return !(a <= b);
    }
    inline auto operator>=(const Str &a, const Str &b) -> bool
    {
        return !(a < b);
    }

    inline auto StrLess(const Str &a, const Str &b) -> bool
    {
        return a < b;
    }

    inline auto StrHash(const Str &s) -> std::size_t
    {
        static std::hash<std::string> hash_;
        ReadGuard rg(s.ExposeLatch());
        return hash_(s.data_);
    }

} // namespace rds

#endif