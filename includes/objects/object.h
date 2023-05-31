#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <util.h>
#include <optional>
#include <deque>

namespace rds
{

    enum class ObjectType
    {
        OBJ,
        STR,
        LIST,
        HASH,
        SET,
        ZSET,
        EXPIRE_ENTRY,
        UNKNOWN
    };

    inline auto ObjectTypeToChar(ObjectType otyp) -> char
    {
        char ret;
        switch (otyp)
        {
        case ObjectType::OBJ:
            ret = 0;
            break;
        case ObjectType::STR:
            ret = 1;
            break;
        case ObjectType::LIST:
            ret = 2;
            break;
        case ObjectType::HASH:
            ret = 3;
            break;
        case ObjectType::SET:
            ret = 4;
            break;
        case ObjectType::ZSET:
            ret = 5;
            break;
        case ObjectType::EXPIRE_ENTRY:
            ret = 6;
            break;
        case ObjectType::UNKNOWN:
            assert(0);
            break;
        }
        return ret;
    }

    inline auto CharToObjectType(char c) -> ObjectType
    {
        ObjectType ret_typ = ObjectType::UNKNOWN;
        switch (c)
        {
        case 0:
            ret_typ = ObjectType::OBJ;
            break;
        case 1:
            ret_typ = ObjectType::STR;
            break;
        case 2:
            ret_typ = ObjectType::LIST;
            break;
        case 3:
            ret_typ = ObjectType::HASH;
            break;
        case 4:
            ret_typ = ObjectType::SET;
            break;
        case 5:
            ret_typ = ObjectType::ZSET;
            break;
        case 6:
            ret_typ = ObjectType::EXPIRE_ENTRY;
            break;
        default:
            assert(0);
            break;
        }
        return ret_typ;
    }

    enum class EncodingType
    {
        INT,
        STR_RAW,
        STR_COMPRESS,
        LIST,    // linked list
        ARRAY,   // zip list
        HASHMAP, // ht
        RBTREE,  // skip list
        UNKNOWN
    };

    inline auto EncodingTypeToChar(EncodingType etyp) -> char
    {
        char ret;
        switch (etyp)
        {
        case EncodingType::INT:
            ret = 0;
            break;
        case EncodingType::STR_RAW:
            ret = 1;
            break;
        case EncodingType::STR_COMPRESS:
            ret = 2;
            break;
        case EncodingType::LIST:
            ret = 3;
            break;
        case EncodingType::ARRAY:
            ret = 4;
            break;
        case EncodingType::HASHMAP:
            ret = 5;
            break;
        case EncodingType::RBTREE:
            ret = 6;
            break;
        case EncodingType::UNKNOWN:
            assert(0);
            break;
        }
        return ret;
    }

    inline auto CharToEncodingType(char c) -> EncodingType
    {
        EncodingType etyp = EncodingType::UNKNOWN;
        switch (c)
        {
        case 0:
            etyp = EncodingType::INT;
            break;
        case 1:
            etyp = EncodingType::STR_RAW;
            break;
        case 2:
            etyp = EncodingType::STR_COMPRESS;
            break;
        case 3:
            etyp = EncodingType::LIST;
            break;
        case 4:
            etyp = EncodingType::ARRAY;
            break;
        case 5:
            etyp = EncodingType::HASHMAP;
            break;
        case 6:
            etyp = EncodingType::RBTREE;
            break;
        default:
            assert(0);
            break;
        }
        return etyp;
    }

    class Object
    {
    private:
    protected:
        mutable std::shared_mutex latch_;

    public:
        auto ExposeLatch() const -> std::shared_mutex &
        {
            return latch_;
        }

        virtual auto EncodeValue() const -> std::string
        {
            return {};
        };

        virtual void DecodeValue(std::deque<char> *){};

        virtual auto GetObjectType() const -> ObjectType
        {
            return ObjectType::OBJ;
        }

        virtual auto Fork(const std::string &key) -> std::string { return {}; };

        // virtual auto Size() -> std::size_t { return 0; }

        Object() = default;
        virtual ~Object() = default;
        Object(const Object &) : Object() {}
        Object(Object &&) noexcept : Object() {}
        Object &operator=(const Object &) { return *this; }
        Object &operator=(Object &&) noexcept { return *this; }
    };

} // namespace rds

#endif