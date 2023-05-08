#ifndef __LIST_H__
#define __LIST_H__

#include <list>
#include <objects/object.h>
#include <objects/str.h>
#include <util.h>

namespace rds
{

    class List final : public Object
    {
    private:
        std::list<Str> data_list_;

    public:
        template <typename T, typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
        void PushFront(T &&);
        template <typename T, typename = std::enable_if_t<std::is_same_v<Str, std::decay_t<T>>, void>>
        void PushBack(T &&);
        auto PopFront() -> Str;
        auto PopBack() -> Str;
        auto Index(std::size_t) const -> Str;
        auto Len() const -> std::size_t;
        auto Rem(const Str &) -> bool;
        void Trim(std::size_t, std::size_t);

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> &) override;

        CLASS_DEFAULT_DECLARE(List);
    };
} // namespace rds

#endif