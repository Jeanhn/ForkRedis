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
        EXPIRE_ENTRY
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
        }
        return ret;
    }

    inline auto CharToObjectType(char c) -> ObjectType
    {
        ObjectType otyp;
        switch (c)
        {
        case 0:
            otyp = ObjectType::OBJ;
            break;
        case 1:
            otyp = ObjectType::STR;
            break;
        case 2:
            otyp = ObjectType::LIST;
            break;
        case 3:
            otyp = ObjectType::HASH;
            break;
        case 4:
            otyp = ObjectType::SET;
            break;
        case 5:
            otyp = ObjectType::ZSET;
            break;
        case 6:
            otyp = ObjectType::EXPIRE_ENTRY;
            break;
        default:
            assert(0);
            break;
        }
        return otyp;
    }

    enum class EncodingType
    {
        INT,
        STR_RAW,
        STR_COMPRESS,
        LIST,    // linked list
        ARRAY,   // zip list
        HASHMAP, // ht
        RBTREE   // skip list
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
        }
        return ret;
    }

    inline auto CharToEncodingType(char c) -> EncodingType
    {
        EncodingType etyp;
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
        ObjectType obj_type_{ObjectType::OBJ};

    protected:
    public:
        virtual auto EncodeValue() const -> std::string = 0;

        virtual void DecodeValue(std::deque<char> &) = 0;

        virtual auto GetObjectType() const -> ObjectType
        {
            return ObjectType::OBJ;
        }

        virtual ~Object() = default;

        CLASS_DECLARE_without_destructor(Object);
    };

} // namespace rds

#endif