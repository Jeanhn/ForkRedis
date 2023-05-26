#include <objects/str.h>
#include <cstring>
#include <database/rdb.h>

namespace rds
{
    void Str::TypeCheck()
    {
        for (auto it = data_.cbegin(); it != data_.cend(); it++)
        {
            if (*it < '0' || *it > '9')
            {
                encoding_type_ = EncodingType::STR_RAW;
                return;
            }
        }
        encoding_type_ = EncodingType::INT;
    }

    auto Str::GetRaw() const -> std::string
    {
        return data_;
    }

    void Str::Set(std::string data)
    {
        data_ = std::move(data);
        TypeCheck();
    }

    auto Str::Append(std::string data) -> std::size_t
    {
        data_.append(std::move(data));
        TypeCheck();
        return data_.size();
    }

    auto Str::IncrBy(int delta) -> std::string
    {
        if (encoding_type_ != EncodingType::INT)
        {
            return {};
        }
        int raw_int = std::stoi(data_);
        raw_int += delta;
        data_ = std::to_string(raw_int);
        return data_;
    }

    auto Str::DecrBy(int delta) -> std::string
    {
        if (encoding_type_ != EncodingType::INT)
        {
            return {};
        }
        int raw_int = std::stoi(data_);
        raw_int -= delta;
        data_ = std::to_string(raw_int);
        return data_;
    }

    auto Str::Len() const -> std::size_t
    {
        return data_.size();
    }

    auto Str::Empty() const -> bool
    {
        return data_.empty();
    }

    // template <typename T, typename =
    //                           std::enable_if_t<std::is_same_v<std::string, std::decay_t<T>>, void>>
    // Str::Str(T &&data) : data_(std::forward<T>(data))
    // {
    // }

    auto Str::GetObjectType() const -> ObjectType
    {
        return ObjectType::STR;
    }

    /*
    char obj-type
    [size_t len_after_compress]/[size_t len_origin]
    [int/string]value
     */
    auto Str::EncodeValue() const -> std::string
    {
        std::string ret;
        // encode-type: int or string or compress-string
        char t = EncodingTypeToChar(encoding_type_);

        assert(encoding_type_ == EncodingType::STR_RAW || encoding_type_ == EncodingType::INT || encoding_type_ == EncodingType::STR_COMPRESS);

        // if str [len] or [len len-before-compress]
        if (encoding_type_ == EncodingType::STR_RAW)
        {
            if (DefineCompress())
            {
                t = EncodingTypeToChar(EncodingType::STR_COMPRESS);
            }
            ret.push_back(t);
            std::string res;
            if (DefineCompress())
            {
                std::string cprs = Compress(data_);
                std::size_t len = cprs.size();
                ret.append(BitsToString(len));
                res = std::move(cprs);
            }
            else
            {
                res = data_;
            }
            std::size_t len = data_.size();
            ret.append(BitsToString(len));
            ret.append(res);
            return ret;
        }

        // int
        ret.push_back(t);
        int data = std::stoi(data_);
        ret.append(BitsToString(data));
        return ret;
    }

    auto Str::GetEncodingType() const -> EncodingType
    {
        return encoding_type_;
    }

    void Str::DecodeValue(std::deque<char> *source)
    {
        EncodingType etyp = CharToEncodingType(source->front());
        source->pop_front();

        assert(etyp == EncodingType::INT || etyp == EncodingType::STR_RAW || etyp == EncodingType::STR_COMPRESS);

        data_.clear();

        if (etyp == EncodingType::INT)
        {
            encoding_type_ = EncodingType::INT;
            data_ = std::to_string(PeekInt(source));
            return;
        }
        encoding_type_ = EncodingType::STR_RAW;
        if (etyp == EncodingType::STR_RAW)
        {
            size_t size = PeekSize(source);
            data_ = PeekString(source, size);
        }
        else if (etyp == EncodingType::STR_COMPRESS)
        {
            size_t size_compress = PeekSize(source);
#ifndef NDEBUG
            size_t size_origin = PeekSize(source);
#else
            PeekSize(source);
#endif
            data_ = Decompress(PeekString(source, size_compress));
            assert(size_origin == data_.size());
        }
    }

} // namespace rds
