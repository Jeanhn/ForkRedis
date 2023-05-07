#include <objects/str.h>
#include <cstring>
#include <database/rdb.h>

namespace rds
{

    auto Str::GetRaw() -> std::string &
    {
        return data_;
    }

    void Str::Set(const std::string &data)
    {
        data_ = data;
    }

    void Str::Set(std::string &&data)
    {
        data_ = std::move(data);
    }

    void Str::Append(const std::string &data)
    {
        data_.append(data);
    }

    void Str::Append(std::string &&data)
    {
        data_.append(std::move(data));
    }

    void Str::IncrBy(int delta)
    {
        int raw_int = std::stoi(data_);
        raw_int += delta;
        data_ = std::to_string(raw_int);
    }

    void Str::DecrBy(int delta)
    {
        int raw_int = std::stoi(data_);
        raw_int -= delta;
        data_ = std::to_string(raw_int);
    }

    auto Str::Len() -> std::size_t
    {
        return data_.size();
    }

    auto Str::Empty() -> bool
    {
        return data_.empty();
    }

    template <typename T, typename =
                              std::enable_if_t<std::is_same_v<std::string, std::decay_t<T>>, void>>
    Str::Str(T &&data) : data_(std::forward<T>(data))
    {
    }

    auto Str::GetObjectType() const -> ObjectType
    {
        return ObjectType::STR;
    }

    auto Compress(const std::string &) -> std::string;
    auto Decompress(const std::string &) -> std::string;

    /*
    char obj-type
    [size_t len_after_compress]/[size_t len_origin]
    [int/string]value
     */
    auto Str::EncodeValue() -> std::string
    {
        std::string ret;
        // encode-type: int or string or compress-string
        char t = EncodingTypeToChar(encoding_type_);

        // if str [len] or [len len-before-compress]
        if (encoding_type_ == EncodingType::STR_RAW)
        {
            if (compress_)
            {
                assert(0);
                t = EncodingTypeToChar(EncodingType::STR_COMPRESS);
            }
            ret.push_back(t);
            std::string res;
            if (compress_)
            {
                assert(0);
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

    void Str::DecodeValue(std::deque<char> &source)
    {
        EncodingType etyp = CharToEncodingType(source.front());
        source.pop_front();

        assert(etyp == EncodingType::INT || etyp == EncodingType::STR_RAW);
        // assert(etyp == EncodingType::STR_COMPRESS);

        if (etyp == EncodingType::INT)
        {
            data_ = std::to_string(PeekInt(source));
        }
        else if (etyp == EncodingType::STR_RAW)
        {
            size_t size = PeekSize(source);
            data_ = PeekString(source, size);
        }
        else if (etyp == EncodingType::STR_COMPRESS)
        {
            size_t size_compress = PeekSize(source);
            /* size_t size_origin =  */ PeekSize(source);
            data_ = Decompress(PeekString(source, size_compress));
        }
    }

} // namespace rds
