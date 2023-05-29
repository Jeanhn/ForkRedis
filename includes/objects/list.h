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
        auto PushFront(Str str) -> std::size_t; // return len of list after push
        auto PushBack(Str str) -> std::size_t;
        auto PopFront() -> Str;
        auto PopBack() -> Str;
        auto Index(int idx) const -> Str; // return "Str" or (nil)
        auto Len() const -> std::size_t;
        auto Rem(int count, const Str &value) -> std::size_t;
        auto Set(int index, const Str &value) -> bool;
        auto Trim(int, int) -> bool; // if success, return "OK"

        auto GetObjectType() const -> ObjectType override;
        auto EncodeValue() const -> std::string override;
        void DecodeValue(std::deque<char> *) override;

        auto Fork(const std::string &key) -> std::string;

        CLASS_DECLARE_special_copy_move(List);
    };
} // namespace rds

#endif